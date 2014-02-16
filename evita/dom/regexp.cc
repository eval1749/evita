// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/regexp.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "evita/dom/buffer.h"
#include "evita/dom/document.h"
#include "evita/dom/range.h"
#include "evita/dom/regexp_init.h"
#include "evita/dom/script_controller.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/runner.h"
#include "evita/v8_glue/wrapper_info.h"
#include "evita/text/range.h"
#include "regex/IRegex.h"
#include "v8_strings.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// ErrorInfo
//
struct ErrorInfo {
  int error_code;
  int offset;

  ErrorInfo() : error_code(0), offset(0) {
  }
};

base::char16 CharUpcase(base::char16 wch) {
  return static_cast<base::char16>(
      reinterpret_cast<UINT_PTR>(
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
struct RegExp::Match {
  int end;
  base::string16 name;
  int start;

  Match() : end(-1), start(-1) {
  }

  void Reset() {
    end = start = -1;
  }

  void Set(int start, int end) {
    this->start = start;
    this->end = end;
  }
};

//////////////////////////////////////////////////////////////////////
//
// RegExp::RegExpImpl
//
class RegExp::RegExpImpl {
  private: std::vector<uint8_t> blob_;
  private: std::vector<RegExp::Match> matches_;
  private: ::Regex::IRegex* regex_impl_;

  public: RegExpImpl(size_t size, int num_matches);
  public: ~RegExpImpl() = default;

  public: void* blob() { return &blob_[0]; }
  public: const std::vector<RegExp::Match>& matches() const {
    return matches_;
  }
  public: ::Regex::IRegex* regex_impl() { return regex_impl_; }
  public: void set_match_name(int nth, const base::string16& name);
  public: void set_regex_impl(::Regex::IRegex* regex_impl) {
    regex_impl_ = regex_impl;
  }

  public: void ResetMatches();

  DISALLOW_COPY_AND_ASSIGN(RegExpImpl);
};

RegExp::RegExpImpl::RegExpImpl(size_t size, int num_matches)
    : blob_(size), matches_(static_cast<size_t>(num_matches + 1)) {
}

void RegExp::RegExpImpl::set_match_name(int nth, const base::string16& name) {
  matches_[static_cast<size_t>(nth)].name = name;
}

void RegExp::RegExpImpl::ResetMatches() {
  for (auto& match : matches_) {
    match.Reset();
  }
}

//////////////////////////////////////////////////////////////////////
//
// Compiler
//
class RegExp::Compiler : public ::Regex::ICompileContext {
  private: std::unique_ptr<RegExpImpl> regex_;
  private: ErrorInfo error_info_;

  public: Compiler() = default;
  public: virtual ~Compiler() = default;

  public: const ErrorInfo& error_info() const {
    return error_info_;
  }

  public: RegExpImpl* Compile(const base::string16& source,
                              const RegExpInit& init_dict);

  // RegExp::ICompileContext
  private: virtual void* AllocRegex(size_t cb, int num_matches) override;
  private: virtual bool SetCapture(int iNth, const char16* pwsz) override;
  private: virtual void SetError(int nPosn, int nError) override;

    DISALLOW_COPY_AND_ASSIGN(Compiler);
};

RegExp::RegExpImpl* RegExp::Compiler::Compile(
    const base::string16& source, const RegExpInit& init_dict) {
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

  auto const regex_impl = ::Regex::Compile(this, source.data(),
      static_cast<int>(source.length()), flags);
  if (!regex_impl)
    return nullptr;
  regex_->set_regex_impl(regex_impl);
  return regex_.release();
}

// RegExp::ICompileContext
void* RegExp::Compiler::AllocRegex(size_t size, int num_matches) {
  DCHECK_GE(size, 1u);
  DCHECK_GE(num_matches, 0);
  regex_.reset(new RegExpImpl(size, num_matches));
  return regex_->blob();
}

bool RegExp::Compiler::SetCapture(int nth, const char16* name) {
  regex_->set_match_name(nth, base::string16(name));
  return true;
}

void RegExp::Compiler::SetError(int offset, int error_code) {
  error_info_.error_code = error_code;
  error_info_.offset = offset;
}

//////////////////////////////////////////////////////////////////////
//
// RegExp::BufferMatcher
//
class RegExp::BufferMatcher : public ::Regex::IMatchContext {
  private: text::Buffer* buffer_;
  private: int end_;
  private: RegExp::RegExpImpl* regex_;
  private: int start_;

  public: BufferMatcher(RegExp::RegExpImpl* regex, text::Buffer* buffer,
                        int start, int end);
  public: virtual ~BufferMatcher();

  // RegExp::IMatchContext
  // [B]
  private: virtual bool BackwardFindCharCi(char16, Posn*, Posn) const override;
  private: virtual bool BackwardFindCharCs(char16, Posn*, Posn) const override;

  // [F]
  private: virtual bool ForwardFindCharCi(char16, Posn*, Posn) const override;
  private: virtual bool ForwardFindCharCs(char16, Posn*, Posn) const override;

  // [G]
  private: virtual bool GetCapture(int index, Posn*, Posn*) const override;
  private: virtual char16 GetChar(Posn lPosn) const override;
  private: virtual Posn GetEnd() const override { return end_; }
  private: virtual void GetInfo(
      ::Regex::SourceInfo* source_info) const override;
  private: virtual Posn GetStart() const override { return start_; }

  // [R]
  private: virtual void ResetCapture(int) override;
  private: virtual void ResetCaptures() override;

  // [S]
  private: virtual void SetCapture(int, Posn, Posn) override;
  private: virtual bool StringEqCi(const char16*, int, Posn) const override;
  private: virtual bool StringEqCs(const char16*, int, Posn) const override;

  DISALLOW_COPY_AND_ASSIGN(BufferMatcher);
};

RegExp::BufferMatcher::BufferMatcher(RegExpImpl* regex, text::Buffer* buffer,
                                    int start, int end)
    : buffer_(buffer), end_(end), regex_(regex), start_(start) { 
}

RegExp::BufferMatcher::~BufferMatcher() {
}

// RegExp::IMatchContext
// [B]
bool RegExp::BufferMatcher::BackwardFindCharCi(char16 wchFind,
                                              Posn* inout_lPosn,
                                              Posn lStop) const {
  text::Buffer::EnumCharRev::Arg oArg(buffer_, *inout_lPosn, lStop);
  foreach (text::Buffer::EnumCharRev, oEnum, oArg) {
    if (CharEqCi(oEnum.Get(), wchFind)) {
      *inout_lPosn = oEnum.GetPosn();
      return true;
    }
  }
  return false;
}

bool RegExp::BufferMatcher::BackwardFindCharCs(char16 wchFind,
                                              Posn* inout_lPosn,
                                              Posn lStop) const {
  text::Buffer::EnumCharRev::Arg oArg(buffer_, *inout_lPosn, lStop);
  foreach (text::Buffer::EnumCharRev, oEnum, oArg) {
    if (CharEqCs(oEnum.Get(), wchFind)) {
      *inout_lPosn = oEnum.GetPosn();
      return true;
    }
  }
  return false;
}

bool RegExp::BufferMatcher::ForwardFindCharCi(char16 wchFind,
                                             Posn* inout_lPosn,
                                             Posn lStop) const {
  text::Buffer::EnumChar::Arg oArg(buffer_, *inout_lPosn,
                                   lStop);
  foreach (text::Buffer::EnumChar, oEnum, oArg) {
    if (CharEqCi(oEnum.Get(), wchFind)) {
      *inout_lPosn = oEnum.GetPosn();
      return true;
    }
  }
  return false;
}

// [F]
bool RegExp::BufferMatcher::ForwardFindCharCs(char16 wchFind,
                                             Posn* inout_lPosn,
                                             Posn lStop) const {
  text::Buffer::EnumChar::Arg oArg(buffer_, *inout_lPosn, lStop);
  foreach (text::Buffer::EnumChar, oEnum, oArg) {
    if (CharEqCs(oEnum.Get(), wchFind)) {
      *inout_lPosn = oEnum.GetPosn();
      return true;
    }
  }
  return false;
}

// [G]
bool RegExp::BufferMatcher::GetCapture(int nth, Posn* out_lStart,
                                       Posn* out_lEnd) const {
  auto const index = static_cast<size_t>(nth);
  if (index >= regex_->matches().size())
    return false;
  auto& match = regex_->matches()[index];
  *out_lStart = match.start;
  *out_lEnd = match.end;
  return true;
}

char16 RegExp::BufferMatcher::GetChar(Posn lPosn) const {
  return buffer_->GetCharAt(lPosn);
}

void RegExp::BufferMatcher::GetInfo(::Regex::SourceInfo* p) const {
  p->m_lStart = 0;
  p->m_lEnd = buffer_->GetEnd();
  p->m_lScanStart = start_;
  p->m_lScanEnd = end_;
}

// [R]
void RegExp::BufferMatcher::ResetCapture(int nth) {
  auto const index = static_cast<size_t>(nth);
  if (index >= regex_->matches().size())
    return;
  const_cast<Match*>(&regex_->matches()[index])->Reset();
}

void RegExp::BufferMatcher::ResetCaptures() {
  regex_->ResetMatches();
}

// [S]
void RegExp::BufferMatcher::SetCapture(int nth, Posn start, Posn end) {
  auto const index = static_cast<size_t>(nth);
  if (index >= regex_->matches().size())
    return;
  const_cast<Match*>(&regex_->matches()[index])->Set(start, end);
}

bool RegExp::BufferMatcher::StringEqCi(const char16* pwchStart, int cwch,
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

bool RegExp::BufferMatcher::StringEqCs(const char16* pwchStart, int cwch,
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
// RegExpClass
//
class RegExp::RegExpClass : public v8_glue::WrapperInfo {
  public: RegExpClass(const char* name)
      : v8_glue::WrapperInfo(name) {
  }
  public: ~RegExpClass() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &RegExpClass::NewRegex);
  }

  private: static RegExp* NewRegex(const base::string16 source,
      v8_glue::Optional<v8::Handle<v8::Object>> opt_dict) {
    RegExpInit init_dict;
    if (!init_dict.Init(opt_dict.value))
      return nullptr;

    RegExp::Compiler compiler;
    auto const regex = compiler.Compile(source, init_dict);
    if (!regex) {
      auto const error_info = compiler.error_info();
      ScriptController::instance()->ThrowError(base::StringPrintf(
          "Failed to compile regex with error code %d at offset %d",
          error_info.error_code, error_info.offset));
      return nullptr;
    }

    return new RegExp(regex, source, init_dict);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("backward", &RegExp::backward)
        .SetProperty("global", &RegExp::global)
        .SetProperty("ignoreCase", &RegExp::ignore_case)
        .SetProperty("matchExact", &RegExp::match_exact)
        .SetProperty("matchWord", &RegExp::match_word)
        .SetProperty("multiline", &RegExp::multiline)
        .SetProperty("source", &RegExp::source)
        .SetProperty("sticky", &RegExp::sticky);
  }

  DISALLOW_COPY_AND_ASSIGN(RegExpClass);
};

//////////////////////////////////////////////////////////////////////
//
// Regex
//
DEFINE_SCRIPTABLE_OBJECT(RegExp, RegExp::RegExpClass);

RegExp::RegExp(RegExpImpl* regex, const base::string16& source,
             const RegExpInit& init_dict)
    : backward_(init_dict.backward()), global_(init_dict.global()),
      ignore_case_(init_dict.ignore_case()),
      match_exact_(init_dict.match_exact()),
      match_word_(init_dict.match_word()),
      multiline_(init_dict.multiline()), regex_(regex), source_(source),
      sticky_(init_dict.sticky()) {
}

RegExp::~RegExp() {
}

v8::Handle<v8::Value> RegExp::ExecuteOnDocument(Document* document, int start,
                                               int end) {
  auto const runner = ScriptController::instance()->runner();
  auto const isolate = runner->isolate();
  BufferMatcher matcher(regex_.get(), document->buffer(), start, end);
  v8_glue::Runner::EscapableHandleScope runner_scope(runner);
  if (!::Regex::StartMatch(regex_->regex_impl(), &matcher))
    return runner_scope.Escape(v8::Local<v8::Value>::New(isolate,
        v8::Null(isolate)));
  return runner_scope.Escape(MakeMatchArray(regex_->matches()));
}

v8::Local<v8::Value> RegExp::MakeMatchArray(
    const std::vector<Match>& matches) {
  auto const runner = ScriptController::instance()->runner();
  auto const isolate = runner->isolate();
  auto const js_matches = v8::Array::New(isolate,
                                          static_cast<int>(matches.size()));

  // Editor.RegExp.Match
  auto const js_match_class = runner->global()->Get(
      v8Strings::Editor.Get(isolate))->ToObject()->Get(
          v8Strings::RegExp.Get(isolate))->ToObject()->Get(
              v8Strings::Match.Get(isolate));

  auto index = 0u;
  for (const auto& match : matches) {
    auto const js_match = runner->CallAsConstructor(js_match_class)->
        ToObject();
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

}   // namespace dom
