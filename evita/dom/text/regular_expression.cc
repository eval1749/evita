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
  bool SetCapture(int iNth, const char16* pwsz) final;
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

bool RegularExpression::Compiler::SetCapture(int nth, const char16* name) {
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
  bool BackwardFindCharCi(char16, Posn*, Posn) const final;
  bool BackwardFindCharCs(char16, Posn*, Posn) const final;
  bool ForwardFindCharCi(char16, Posn*, Posn) const final;
  bool ForwardFindCharCs(char16, Posn*, Posn) const final;
  bool GetCapture(int index, Posn*, Posn*) const final;
  char16 GetChar(Posn lPosn) const final;
  Posn GetEnd() const final { return end_; }
  void GetInfo(::Regex::SourceInfo* source_info) const final;
  Posn GetStart() const final { return start_; }
  void ResetCapture(int index) final;
  void ResetCaptures() final;
  void SetCapture(int, Posn, Posn) final;
  bool StringEqCi(const char16*, int, Posn) const final;
  bool StringEqCs(const char16*, int, Posn) const final;

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
bool RegularExpression::BufferMatcher::BackwardFindCharCi(char16 wchFind,
                                                          Posn* inout_lPosn,
                                                          Posn lStop) const {
  text::Buffer::EnumCharRev::Arg oArg(buffer_, *inout_lPosn, lStop);
  for (text::Buffer::EnumCharRev oEnum(oArg); !oEnum.AtEnd(); oEnum.Next()) {
    if (CharEqCi(oEnum.Get(), wchFind)) {
      *inout_lPosn = oEnum.GetPosn();
      return true;
    }
  }
  return false;
}

bool RegularExpression::BufferMatcher::BackwardFindCharCs(char16 wchFind,
                                                          Posn* inout_lPosn,
                                                          Posn lStop) const {
  text::Buffer::EnumCharRev::Arg oArg(buffer_, *inout_lPosn, lStop);
  for (text::Buffer::EnumCharRev oEnum(oArg); !oEnum.AtEnd(); oEnum.Next()) {
    if (CharEqCs(oEnum.Get(), wchFind)) {
      *inout_lPosn = oEnum.GetPosn();
      return true;
    }
  }
  return false;
}

bool RegularExpression::BufferMatcher::ForwardFindCharCi(char16 wchFind,
                                                         Posn* inout_lPosn,
                                                         Posn lStop) const {
  text::Buffer::EnumChar::Arg oArg(buffer_, *inout_lPosn, lStop);
  for (text::Buffer::EnumChar oEnum(oArg); !oEnum.AtEnd(); oEnum.Next()) {
    if (CharEqCi(oEnum.Get(), wchFind)) {
      *inout_lPosn = oEnum.GetPosn();
      return true;
    }
  }
  return false;
}

// [F]
bool RegularExpression::BufferMatcher::ForwardFindCharCs(char16 wchFind,
                                                         Posn* inout_lPosn,
                                                         Posn lStop) const {
  text::Buffer::EnumChar::Arg oArg(buffer_, *inout_lPosn, lStop);
  for (text::Buffer::EnumChar oEnum(oArg); !oEnum.AtEnd(); oEnum.Next()) {
    if (CharEqCs(oEnum.Get(), wchFind)) {
      *inout_lPosn = oEnum.GetPosn();
      return true;
    }
  }
  return false;
}

// [G]
bool RegularExpression::BufferMatcher::GetCapture(int nth,
                                                  Posn* out_lStart,
                                                  Posn* out_lEnd) const {
  auto const index = static_cast<size_t>(nth);
  if (index >= regex_->matches().size())
    return false;
  auto& match = regex_->matches()[index];
  *out_lStart = match.start;
  *out_lEnd = match.end;
  return true;
}

char16 RegularExpression::BufferMatcher::GetChar(Posn lPosn) const {
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
                                                  Posn start,
                                                  Posn end) {
  auto const index = static_cast<size_t>(nth);
  if (index >= regex_->matches().size())
    return;
  const_cast<Match*>(&regex_->matches()[index])->Set(start, end);
}

bool RegularExpression::BufferMatcher::StringEqCi(const char16* pwchStart,
                                                  int cwch,
                                                  Posn lPosn) const {
  text::Buffer::EnumChar::Arg oArg(buffer_, lPosn);
  text::Buffer::EnumChar oEnum(oArg);
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

bool RegularExpression::BufferMatcher::StringEqCs(const char16* pwchStart,
                                                  int cwch,
                                                  Posn lPosn) const {
  text::Buffer::EnumChar::Arg oArg(buffer_, lPosn);
  text::Buffer::EnumChar oEnum(oArg);
  const char16* pwchEnd = pwchStart + cwch;
  for (const char16* pwch = pwchStart; pwch < pwchEnd; pwch++) {
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
