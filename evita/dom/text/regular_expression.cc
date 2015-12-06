// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/text/regular_expression.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "evita/bindings/v8_glue_RegExpInit.h"
#include "evita/dom/text/document.h"
#include "evita/dom/text/range.h"
#include "evita/dom/script_host.h"
#include "evita/v8_glue/runner.h"
#include "evita/text/buffer.h"
#include "evita/text/range.h"
#include "regex/regex.h"
#include "v8_strings.h"  // NOLINT(build/include)

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// ErrorInfo
//
struct ErrorInfo final {
  int error_code;
  int offset;

  ErrorInfo() : error_code(0), offset(0) {}
};

class EnumChar final {
 public:
  struct Arg final {
    text::Posn end_;
    text::Posn offset_;
    const text::Buffer* buffer_;

    Arg(const text::Buffer* pBuffer, text::Posn lPosn, text::Posn lEnd)
        : end_(lEnd), offset_(lPosn), buffer_(pBuffer) {}

    Arg(const text::Buffer* pBuffer, text::Posn lPosn)
        : end_(pBuffer->GetEnd()), offset_(lPosn), buffer_(pBuffer) {}
  };

  explicit EnumChar(const text::Buffer* pBuffer)
      : end_(pBuffer->GetEnd()), offset_(0), buffer_(pBuffer) {
    DCHECK(buffer_->IsValidRange(offset_, end_));
  }

  explicit EnumChar(Arg oArg)
      : end_(oArg.end_), offset_(oArg.offset_), buffer_(oArg.buffer_) {
    DCHECK(buffer_->IsValidRange(offset_, end_));
  }

  bool AtEnd() const { return offset_ >= end_; }

  base::char16 Get() const {
    DCHECK(!AtEnd());
    return buffer_->GetCharAt(offset_);
  }

  text::Posn GetPosn() const { return offset_; }

  const css::Style& GetStyle() const {
    DCHECK(!AtEnd());
    return buffer_->GetStyleAt(offset_);
  }

  text::Posn GoTo(text::Posn lPosn) { return offset_ = lPosn; }
  void Next() {
    DCHECK(!AtEnd());
    offset_ += 1;
  }
  void Prev() { offset_ -= 1; }

  void SyncEnd() { end_ = buffer_->GetEnd(); }

 private:
  text::Posn end_;
  text::Posn offset_;
  const text::Buffer* buffer_;

  DISALLOW_COPY_AND_ASSIGN(EnumChar);
};

class EnumCharRev final {
 public:
  struct Arg {
    text::Posn offset_;
    text::Posn start_;
    const text::Buffer* buffer_;
    Arg(const text::Buffer* pBuffer, text::Posn lPosn, text::Posn lStart = 0)
        : offset_(lPosn), start_(lStart), buffer_(pBuffer) {}
  };

  explicit EnumCharRev(Arg oArg)
      : offset_(oArg.offset_), start_(oArg.start_), buffer_(oArg.buffer_) {}

  bool AtEnd() const { return offset_ <= start_; }

  base::char16 Get() const {
    DCHECK(!AtEnd());
    return buffer_->GetCharAt(offset_ - 1);
  }

  text::Posn GetPosn() const {
    DCHECK(!AtEnd());
    return offset_;
  }

  const css::Style& GetStyle() const {
    DCHECK(!AtEnd());
    return buffer_->GetStyleAt(offset_ - 1);
  }

  void Next() {
    DCHECK(!AtEnd());
    --offset_;
  }
  void Prev() { ++offset_; }

 private:
  text::Posn offset_;
  text::Posn start_;
  const text::Buffer* buffer_;

  DISALLOW_COPY_AND_ASSIGN(EnumCharRev);
};

base::char16 CharUpcase(base::char16 wch) {
  return static_cast<base::char16>(reinterpret_cast<UINT_PTR>(
      ::CharUpper(reinterpret_cast<base::char16*>(wch))));
}

bool CharEqCi(base::char16 wch1, base::char16 wch2) {
  return wch1 == wch2 || CharUpcase(wch1) == CharUpcase(wch2);
}

bool CharEqCs(base::char16 wch1, base::char16 wch2) {
  return wch1 == wch2;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Match
//
struct RegularExpression::Match final {
  int end;
  base::string16 name;
  int start;

  Match() : end(-1), start(-1) {}

  void Reset() { end = start = -1; }

  void Set(int start_in, int end_in) {
    start = start_in;
    end = end_in;
  }
};

//////////////////////////////////////////////////////////////////////
//
// RegularExpression::RegularExpressionImpl
//
class RegularExpression::RegularExpressionImpl final {
 public:
  RegularExpressionImpl(size_t size, int num_matches);
  ~RegularExpressionImpl() = default;

  void* blob() { return &blob_[0]; }
  const std::vector<RegularExpression::Match>& matches() const {
    return matches_;
  }
  ::Regex::IRegex* regex_impl() { return regex_impl_; }
  void set_match_name(int nth, const base::string16& name);
  void set_regex_impl(::Regex::IRegex* regex_impl) { regex_impl_ = regex_impl; }

  void ResetMatches();

 private:
  std::vector<uint8_t> blob_;
  std::vector<RegularExpression::Match> matches_;
  ::Regex::IRegex* regex_impl_;

  DISALLOW_COPY_AND_ASSIGN(RegularExpressionImpl);
};

RegularExpression::RegularExpressionImpl::RegularExpressionImpl(size_t size,
                                                                int num_matches)
    : blob_(size), matches_(static_cast<size_t>(num_matches + 1)) {}

void RegularExpression::RegularExpressionImpl::set_match_name(
    int nth,
    const base::string16& name) {
  matches_[static_cast<size_t>(nth)].name = name;
}

void RegularExpression::RegularExpressionImpl::ResetMatches() {
  for (auto& match : matches_) {
    match.Reset();
  }
}

//////////////////////////////////////////////////////////////////////
//
// Compiler
//
class RegularExpression::Compiler final : public ::Regex::ICompileContext {
 public:
  Compiler() = default;
  ~Compiler() = default;

  const ErrorInfo& error_info() const { return error_info_; }

  RegularExpressionImpl* Compile(const base::string16& source,
                                 const RegExpInit& init_dict);

 private:
  // RegularExpression::ICompileContext
  void* AllocRegex(size_t cb, int num_matches) final;
  bool SetCapture(int iNth, const base::char16* pwsz) final;
  void SetError(int nPosn, int nError) final;

  std::unique_ptr<RegularExpressionImpl> regex_;
  ErrorInfo error_info_;

  DISALLOW_COPY_AND_ASSIGN(Compiler);
};

RegularExpression::RegularExpressionImpl* RegularExpression::Compiler::Compile(
    const base::string16& source,
    const RegExpInit& init_dict) {
  auto flags = 0;
  if (init_dict.backward())
    flags |= ::Regex::Option_Backward;
  if (init_dict.ignore_case())
    flags |= ::Regex::Option_IgnoreCase;
  if (init_dict.match_exact())
    flags |= ::Regex::Option_ExactString;
  else
    flags |= ::Regex::Option_Unicode;
  if (init_dict.match_word())
    flags |= ::Regex::Option_ExactWord;
  if (init_dict.multiline())
    flags |= ::Regex::Option_Multiline;

  auto const regex_impl = ::Regex::Compile(
      this, source.data(), static_cast<int>(source.length()), flags);
  if (!regex_impl)
    return nullptr;
  regex_->set_regex_impl(regex_impl);
  return regex_.release();
}

// RegularExpression::ICompileContext
void* RegularExpression::Compiler::AllocRegex(size_t size, int num_matches) {
  DCHECK_GE(size, 1u);
  DCHECK_GE(num_matches, 0);
  regex_.reset(new RegularExpressionImpl(size, num_matches));
  return regex_->blob();
}

bool RegularExpression::Compiler::SetCapture(int nth,
                                             const base::char16* name) {
  regex_->set_match_name(nth, base::string16(name));
  return true;
}

void RegularExpression::Compiler::SetError(int offset, int error_code) {
  error_info_.error_code = error_code;
  error_info_.offset = offset;
}

//////////////////////////////////////////////////////////////////////
//
// RegularExpression::BufferMatcher
//
class RegularExpression::BufferMatcher final : public ::Regex::IMatchContext {
 public:
  BufferMatcher(RegularExpression::RegularExpressionImpl* regex,
                text::Buffer* buffer,
                int start,
                int end);
  ~BufferMatcher();

 private:
  // RegularExpression::IMatchContext
  bool BackwardFindCharCi(base::char16, text::Posn*, text::Posn) const final;
  bool BackwardFindCharCs(base::char16, text::Posn*, text::Posn) const final;
  bool ForwardFindCharCi(base::char16, text::Posn*, text::Posn) const final;
  bool ForwardFindCharCs(base::char16, text::Posn*, text::Posn) const final;
  bool GetCapture(int index, text::Posn*, text::Posn*) const final;
  base::char16 GetChar(text::Posn lPosn) const final;
  text::Posn GetEnd() const final { return end_; }
  void GetInfo(::Regex::SourceInfo* source_info) const final;
  text::Posn GetStart() const final { return start_; }
  void ResetCapture(int index) final;
  void ResetCaptures() final;
  void SetCapture(int, text::Posn, text::Posn) final;
  bool StringEqCi(const base::char16*, int, text::Posn) const final;
  bool StringEqCs(const base::char16*, int, text::Posn) const final;

  text::Buffer* buffer_;
  int end_;
  RegularExpression::RegularExpressionImpl* regex_;
  int start_;

  DISALLOW_COPY_AND_ASSIGN(BufferMatcher);
};

RegularExpression::BufferMatcher::BufferMatcher(RegularExpressionImpl* regex,
                                                text::Buffer* buffer,
                                                int start,
                                                int end)
    : buffer_(buffer), end_(end), regex_(regex), start_(start) {}

RegularExpression::BufferMatcher::~BufferMatcher() {}

// RegularExpression::IMatchContext
// [B]
bool RegularExpression::BufferMatcher::BackwardFindCharCi(
    base::char16 wchFind,
    text::Posn* inout_lPosn,
    text::Posn lStop) const {
  EnumCharRev::Arg oArg(buffer_, *inout_lPosn, lStop);
  for (EnumCharRev oEnum(oArg); !oEnum.AtEnd(); oEnum.Next()) {
    if (CharEqCi(oEnum.Get(), wchFind)) {
      *inout_lPosn = oEnum.GetPosn();
      return true;
    }
  }
  return false;
}

bool RegularExpression::BufferMatcher::BackwardFindCharCs(
    base::char16 wchFind,
    text::Posn* inout_lPosn,
    text::Posn lStop) const {
  EnumCharRev::Arg oArg(buffer_, *inout_lPosn, lStop);
  for (EnumCharRev oEnum(oArg); !oEnum.AtEnd(); oEnum.Next()) {
    if (CharEqCs(oEnum.Get(), wchFind)) {
      *inout_lPosn = oEnum.GetPosn();
      return true;
    }
  }
  return false;
}

bool RegularExpression::BufferMatcher::ForwardFindCharCi(
    base::char16 wchFind,
    text::Posn* inout_lPosn,
    text::Posn lStop) const {
  EnumChar::Arg oArg(buffer_, *inout_lPosn, lStop);
  for (EnumChar oEnum(oArg); !oEnum.AtEnd(); oEnum.Next()) {
    if (CharEqCi(oEnum.Get(), wchFind)) {
      *inout_lPosn = oEnum.GetPosn();
      return true;
    }
  }
  return false;
}

// [F]
bool RegularExpression::BufferMatcher::ForwardFindCharCs(
    base::char16 wchFind,
    text::Posn* inout_lPosn,
    text::Posn lStop) const {
  EnumChar::Arg oArg(buffer_, *inout_lPosn, lStop);
  for (EnumChar oEnum(oArg); !oEnum.AtEnd(); oEnum.Next()) {
    if (CharEqCs(oEnum.Get(), wchFind)) {
      *inout_lPosn = oEnum.GetPosn();
      return true;
    }
  }
  return false;
}

// [G]
bool RegularExpression::BufferMatcher::GetCapture(int nth,
                                                  text::Posn* out_lStart,
                                                  text::Posn* out_lEnd) const {
  auto const index = static_cast<size_t>(nth);
  if (index >= regex_->matches().size())
    return false;
  auto& match = regex_->matches()[index];
  *out_lStart = match.start;
  *out_lEnd = match.end;
  return true;
}

base::char16 RegularExpression::BufferMatcher::GetChar(text::Posn lPosn) const {
  return buffer_->GetCharAt(lPosn);
}

void RegularExpression::BufferMatcher::GetInfo(::Regex::SourceInfo* p) const {
  p->m_lStart = 0;
  p->m_lEnd = buffer_->GetEnd();
  p->m_lScanStart = start_;
  p->m_lScanEnd = end_;
}

// [R]
void RegularExpression::BufferMatcher::ResetCapture(int nth) {
  auto const index = static_cast<size_t>(nth);
  if (index >= regex_->matches().size())
    return;
  const_cast<Match*>(&regex_->matches()[index])->Reset();
}

void RegularExpression::BufferMatcher::ResetCaptures() {
  regex_->ResetMatches();
}

// [S]
void RegularExpression::BufferMatcher::SetCapture(int nth,
                                                  text::Posn start,
                                                  text::Posn end) {
  auto const index = static_cast<size_t>(nth);
  if (index >= regex_->matches().size())
    return;
  const_cast<Match*>(&regex_->matches()[index])->Set(start, end);
}

bool RegularExpression::BufferMatcher::StringEqCi(const base::char16* pwchStart,
                                                  int cwch,
                                                  text::Posn lPosn) const {
  EnumChar::Arg oArg(buffer_, lPosn);
  EnumChar oEnum(oArg);
  auto const pwchEnd = pwchStart + cwch;
  for (auto pwch = pwchStart; pwch < pwchEnd; pwch++) {
    if (oEnum.AtEnd())
      return false;
    if (!CharEqCi(*pwch, oEnum.Get()))
      return false;
    oEnum.Next();
  }
  return true;
}

bool RegularExpression::BufferMatcher::StringEqCs(const base::char16* pwchStart,
                                                  int cwch,
                                                  text::Posn lPosn) const {
  EnumChar::Arg oArg(buffer_, lPosn);
  EnumChar oEnum(oArg);
  const base::char16* pwchEnd = pwchStart + cwch;
  for (const base::char16* pwch = pwchStart; pwch < pwchEnd; pwch++) {
    if (oEnum.AtEnd())
      return false;

    if (!CharEqCs(*pwch, oEnum.Get()))
      return false;

    oEnum.Next();
  }
  return true;
}

//////////////////////////////////////////////////////////////////////
//
// Regex
//
RegularExpression::RegularExpression(RegularExpressionImpl* regex,
                                     const base::string16& source,
                                     const RegExpInit& init_dict)
    : backward_(init_dict.backward()),
      global_(init_dict.global()),
      ignore_case_(init_dict.ignore_case()),
      match_exact_(init_dict.match_exact()),
      match_word_(init_dict.match_word()),
      multiline_(init_dict.multiline()),
      regex_(regex),
      source_(source),
      sticky_(init_dict.sticky()) {}

RegularExpression::~RegularExpression() {}

v8::Handle<v8::Value> RegularExpression::ExecuteOnDocument(Document* document,
                                                           int start,
                                                           int end) {
  auto const runner = ScriptHost::instance()->runner();
  auto const isolate = runner->isolate();
  BufferMatcher matcher(regex_.get(), document->buffer(), start, end);
  v8_glue::Runner::EscapableHandleScope runner_scope(runner);
  if (!::Regex::StartMatch(regex_->regex_impl(), &matcher))
    return runner_scope.Escape(
        v8::Local<v8::Value>::New(isolate, v8::Null(isolate)));
  return runner_scope.Escape(MakeMatchArray(regex_->matches()));
}

v8::Local<v8::Value> RegularExpression::MakeMatchArray(
    const std::vector<Match>& matches) {
  auto const runner = ScriptHost::instance()->runner();
  auto const isolate = runner->isolate();
  auto const js_matches =
      v8::Array::New(isolate, static_cast<int>(matches.size()));

  // Editor.RegularExpression.Match
  auto const js_match_class = runner->global()
                                  ->Get(v8Strings::Editor.Get(isolate))
                                  ->ToObject()
                                  ->Get(v8Strings::RegExp.Get(isolate))
                                  ->ToObject()
                                  ->Get(v8Strings::Match.Get(isolate));

  auto index = 0u;
  for (const auto& match : matches) {
    auto const js_match = runner->CallAsConstructor(js_match_class)->ToObject();
    js_match->Set(v8Strings::name.Get(isolate),
                  gin::ConvertToV8(isolate, match.name));
    js_match->Set(v8Strings::start.Get(isolate),
                  v8::Integer::New(isolate, match.start));
    js_match->Set(v8Strings::end.Get(isolate),
                  v8::Integer::New(isolate, match.end));
    js_matches->Set(index, js_match);
    ++index;
  }

  return js_matches;
}

RegularExpression* RegularExpression::NewRegularExpression(
    const base::string16& source,
    const RegExpInit& options) {
  RegularExpression::Compiler compiler;
  auto const regex = compiler.Compile(source, options);
  if (!regex) {
    auto const error_info = compiler.error_info();
    ScriptHost::instance()->ThrowError(base::StringPrintf(
        "Failed to compile regex with error code %d at offset %d",
        error_info.error_code, error_info.offset));
    return nullptr;
  }

  return new RegularExpression(regex, source, options);
}

RegularExpression* RegularExpression::NewRegularExpression(
    const base::string16& source) {
  return NewRegularExpression(source, RegExpInit());
}

}  // namespace dom
