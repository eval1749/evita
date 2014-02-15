// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/regex_matcher.h"

#include <vector>

#include "base/logging.h"
#include "base/strings/string16.h"
#include "evita/text/buffer.h"
#include "evita/text/range.h"

namespace text {
namespace {

bool CharEqCi(base::char16 wch1, base::char16 wch2) {
  return wch1 == wch2 || ::CharUpcase(wch1) == ::CharUpcase(wch2);
}

bool CharEqCs(base::char16 wch1, base::char16 wch2) {
  return wch1 == wch2;
}

bool StringEqCi(const base::string16& string1, const base::string16& string2) {
  if (string1.length() != string2.length())
    return false;
  auto it1 = string1.begin();
  auto it2 = string2.begin();
  while (it1 != string1.end()) {
    if (!CharEqCi(*it1, *it2))
      return false;
  }
  return true;
}

//////////////////////////////////////////////////////////////////////
//
// Capture
//
class Capture {
  private: base::string16 name_;
  private: text::Range* range_;

  public: Capture();
  public: ~Capture();

  public: const base::string16 name() const { return name_; }
  public: void set_name(const base::string16& name) { name_ = name; }
  public: text::Range* range() const { return range_; }
  public: void set_range(text::Range* range) { range_ = range; }

  public: void Reset();
};

Capture::Capture() : range_(nullptr) {
}

Capture::~Capture() {
}

void Capture::Reset() {
  if (!range_)
    return;
  range_->destroy();
  range_ = nullptr;
}

//////////////////////////////////////////////////////////////////////
//
// CompileContext
//
class CompileContext : public Regex::ICompileContext {
  private: std::vector<uint8_t> regex_;
  private: std::vector<Capture> captures_;
  private: RegexMatcher::ErrorInfo error_info_;

  public: CompileContext();
  public: virtual ~CompileContext();

  public: Capture* capture(int index);
  public: const Capture* capture(const base::string16& name) const;
  public: const RegexMatcher::ErrorInfo& error_info() const {
    return error_info_;
  }

  public: void ResetCaptures();

  // Regex::ICompileContext
  private: virtual void* AllocRegex(size_t cb, int num_captures) override;
  private: virtual bool SetCapture(int iNth, const char16* pwsz) override;
  private: virtual void SetError(int nPosn, int nError) override;
};

CompileContext::CompileContext() {
}

CompileContext::~CompileContext() {
}

Capture* CompileContext::capture(int nth) {
  auto const index = static_cast<size_t>(nth);
  if (index > captures_.size())
    return nullptr;
  return &captures_[index];
}

const Capture* CompileContext::capture(const base::string16& name) const {
  for (auto& capture : captures_) {
    if (StringEqCi(capture.name(), name))
      return &capture;
  }
  return nullptr;
}

void CompileContext::ResetCaptures() {
  for (auto& capture : captures_) {
    capture.Reset();
  }
}

// ICompileContext
void* CompileContext::AllocRegex(size_t cb, int num_captures) {
  DCHECK(captures_.empty());
  DCHECK_GE(cb, 1u);
  DCHECK_GE(num_captures, 0);
  regex_.resize(cb);
  captures_.resize(static_cast<size_t>(num_captures + 1));
  return &regex_[0];
}

bool CompileContext::SetCapture(int nth, const char16* pwsz) {
  auto const index = static_cast<size_t>(nth);
  DCHECK_LT(index, captures_.size());
  captures_[index].set_name(base::string16(pwsz));
  return true;
}

void CompileContext::SetError(int offset, int error_code) {
  error_info_.error_code = error_code;
  error_info_.offset = offset;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// BufferMatchContext
//
class RegexMatcher::BufferMatchContext : public Regex::IMatchContext {
  private: size_t m_num_captures;
  private: text::Range range_;
  private: std::unique_ptr<CompileContext> compile_context_;
  private: Regex::IRegex* regex_;

  public: BufferMatchContext(
      Regex::IRegex* pIRegex,
      std::unique_ptr<CompileContext>&& compile_context,
      text::Buffer* pBuffer,
      Posn lStart,
      Posn lEnd);
  public: virtual ~BufferMatchContext();

  public: text::Range* GetCapture(const base::string16& name) const;
  public: text::Range* GetCapture(int index) const;
  public: text::Range* GetRange() { return &range_; }

  // Regex::IMatchContext
  // [B]
  private: virtual bool BackwardFindCharCi(char16, Posn*, Posn) const override;
  private: virtual bool BackwardFindCharCs(char16, Posn*, Posn) const override;

  // [F]
  private: virtual bool ForwardFindCharCi(char16, Posn*, Posn) const override;
  private: virtual bool ForwardFindCharCs(char16, Posn*, Posn) const override;

  // [G]
  private: virtual bool GetCapture(int index, Posn*, Posn*) const override;
  private: virtual char16 GetChar(Posn lPosn) const override;
  private: virtual Posn GetEnd() const override { return range_.GetEnd(); }
  private: virtual void GetInfo(Regex::SourceInfo* source_info) const override;
  private: virtual Posn GetStart() const override { return range_.GetStart(); }

  // [R]
  private: virtual void ResetCapture(int) override;
  private: virtual void ResetCaptures() override;

  // [S]
  private: virtual void SetCapture(int, Posn, Posn) override;
  private: virtual bool StringEqCi(const char16*, int, Posn) const override;
  private: virtual bool StringEqCs(const char16*, int, Posn) const override;

  DISALLOW_COPY_AND_ASSIGN(BufferMatchContext);
};

RegexMatcher::BufferMatchContext::BufferMatchContext(
    Regex::IRegex* pIRegex,
    std::unique_ptr<CompileContext>&& compile_context,
    text::Buffer* pBuffer, Posn lStart, Posn lEnd)
    : range_(pBuffer, lStart, lEnd),
      compile_context_(std::move(compile_context)),
      regex_(pIRegex) {
}

RegexMatcher::BufferMatchContext::~BufferMatchContext() {
}

text::Range* RegexMatcher::BufferMatchContext::GetCapture(
    const base::string16& name) const {
  auto const capture = compile_context_->capture(name);
  return capture ? capture->range() : nullptr;
}

text::Range* RegexMatcher::BufferMatchContext::GetCapture(int index) const {
  auto const capture = compile_context_->capture(index);
  return capture ? capture->range() : nullptr;
}

// Regex::IMatchContext
// [B]
bool RegexMatcher::BufferMatchContext::BackwardFindCharCi(char16 wchFind,
                                                          Posn* inout_lPosn,
                                                          Posn lStop) const {
  text::Buffer::EnumCharRev::Arg oArg(range_.GetBuffer(), *inout_lPosn,
                                      lStop);
  foreach (text::Buffer::EnumCharRev, oEnum, oArg) {
    if (CharEqCi(oEnum.Get(), wchFind)) {
      *inout_lPosn = oEnum.GetPosn();
      return true;
    }
  }
  return false;
}

bool RegexMatcher::BufferMatchContext::BackwardFindCharCs(char16 wchFind,
                                                          Posn* inout_lPosn,
                                                          Posn lStop) const {
  text::Buffer::EnumCharRev::Arg oArg(range_.GetBuffer(), *inout_lPosn,
                                      lStop);

  foreach (text::Buffer::EnumCharRev, oEnum, oArg) {
    if (CharEqCs(oEnum.Get(), wchFind)) {
      *inout_lPosn = oEnum.GetPosn();
      return true;
    }
  }
  return false;
}

bool RegexMatcher::BufferMatchContext::ForwardFindCharCi(char16 wchFind,
                                                         Posn* inout_lPosn,
                                                         Posn lStop) const {
  text::Buffer::EnumChar::Arg oArg(range_.GetBuffer(), *inout_lPosn,
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
bool RegexMatcher::BufferMatchContext::ForwardFindCharCs(char16 wchFind,
                                                         Posn* inout_lPosn,
                                                         Posn lStop) const {
  text::Buffer::EnumChar::Arg oArg(range_.GetBuffer(), *inout_lPosn, lStop);
  foreach (text::Buffer::EnumChar, oEnum, oArg) {
    if (CharEqCs(oEnum.Get(), wchFind)) {
      *inout_lPosn = oEnum.GetPosn();
      return true;
    }
  }
  return false;
}

// [G]
bool RegexMatcher::BufferMatchContext::GetCapture(int index,
                                                  Regex::Posn* out_lStart,
                                                  Regex::Posn* out_lEnd) const {
  auto const capture = compile_context_->capture(index);
  if (!capture)
    return false;
  auto const range = capture->range();
  if (!range)
    return false;
  *out_lStart = range->GetStart();
  *out_lEnd = range->GetEnd();
  return true;
}

char16 RegexMatcher::BufferMatchContext::GetChar(Posn lPosn) const {
  return range_.GetBuffer()->GetCharAt(lPosn);
}

void RegexMatcher::BufferMatchContext::GetInfo(Regex::SourceInfo* p) const {
  p->m_lStart = 0;
  p->m_lEnd = range_.GetBuffer()->GetEnd();
  p->m_lScanStart = range_.GetStart();
  p->m_lScanEnd = range_.GetEnd();
}

// [R]
void RegexMatcher::BufferMatchContext::ResetCapture(int index) {
  if (auto const capture = compile_context_->capture(index))
    capture->Reset();
}

void RegexMatcher::BufferMatchContext::ResetCaptures() {
  compile_context_->ResetCaptures();
}

// [S]
void RegexMatcher::BufferMatchContext::SetCapture(int index, Posn lStart,
                                                  Posn lEnd) {
  auto const capture = compile_context_->capture(index);
  if (!capture)
    return;
  if (auto const range = capture->range()) {
    range->SetRange(lStart, lEnd);
    return;
  }

  auto const buffer = range_.GetBuffer();
  auto const range = new(buffer->GetHeap()) text::Range(buffer, lStart, lEnd);
  capture->set_range(range);
}

bool RegexMatcher::BufferMatchContext::StringEqCi(const char16* pwchStart,
                                                  int cwch, Posn lPosn) const {
  text::Buffer::EnumChar::Arg oArg(range_.GetBuffer(), lPosn);
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

bool RegexMatcher::BufferMatchContext::StringEqCs(const char16* pwchStart,
                                                  int cwch, Posn lPosn) const {
  text::Buffer::EnumChar::Arg oArg(
      range_.GetBuffer(),
      lPosn);

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
// RegexMatcher
//
RegexMatcher::RegexMatcher(const text::SearchParameters* pSearch,
                           text::Buffer* pBuffer, Posn lStart, Posn lEnd)
    : matched_(false), search_params_(*pSearch), regex_(nullptr) {
  auto rgfFlag = static_cast<int>(Regex::Option_ExactString);
  if (pSearch->IsBackward())
    rgfFlag |= Regex::Option_Backward;

  if (pSearch->IsIgnoreCase())
    rgfFlag |= Regex::Option_IgnoreCase;

  // FIXME 2008-07-08 yosi@msn.com How do we handle text::SearchFlag_MatchWord?
  if (pSearch->IsRegex()) {
    rgfFlag &= ~Regex::Option_ExactString;
    rgfFlag |= Regex::Option_Multiline;
    rgfFlag |= Regex::Option_Unicode;
  } else if (pSearch->IsMatchWord()) {
    rgfFlag |= Regex::Option_ExactWord;
  }

  auto compile_context = std::make_unique<CompileContext>();

  // FIXME 2008-07-08 yosi@msn.com We should not allocate Regex object
  // in Regex::Compiler method.
  regex_ = Regex::Compile(compile_context.get(),
      search_params_.search_text.data(),
      static_cast<int>(search_params_.search_text.length()),
      rgfFlag);
  if (!regex_) {
    error_info_ = compile_context->error_info();
    return;
  }

  match_context_.reset(new BufferMatchContext(regex_,
      std::move(compile_context), pBuffer, lStart, lEnd));
}

RegexMatcher::~RegexMatcher() {
}

bool RegexMatcher::FirstMatch() {
  if (!regex_ || !match_context_)
    return false;
  return matched_ = Regex::StartMatch(regex_, match_context_.get());
}

RegexMatcher::ErrorInfo RegexMatcher::GetErrorInfo() const {
  return error_info_;
}

/// <summary>
/// Retreives captured range of specified position. Position 0 specifies
/// entire matched text. Position 1 specifies the first captur, 2 is
/// second capture and so on.
/// <para>
/// This method returns nullptr in following cases:
/// <list>
/// <item><description>
/// The last match is failed.
/// </description></item>
/// <item><description>
/// Specified number of capture doesn't exist.
/// </description></item>
/// </list>
/// </para>
/// </summary>
/// <param name="pwchName">A capture name</param>
/// <param name="cwchName">A length of capture name</param>
/// <returns>A text::Range of nth capture.</returns>
text::Range* RegexMatcher::GetMatched(int nNth) {
  if (!matched_ || !regex_ || !match_context_)
    return nullptr;
  return match_context_->GetCapture(nNth);
}

text::Range* RegexMatcher::GetMatched(const base::string16& name) {
  if (!matched_ || !regex_ || !match_context_)
    return false;
  return match_context_->GetCapture(name);
}

bool RegexMatcher::NextMatch() {
  if (!matched_ || !match_context_)
    return false;
  return matched_ = Regex::NextMatch(regex_, match_context_.get());
}

class EnumChar {
  private: const char16* m_pwch;
  private: const char16* m_pwchEnd;

  public: struct Arg {
      int m_cwch;
      const char16* m_pwch;

      Arg(const char16* pwch, int cwch) :
          m_cwch(cwch), m_pwch(pwch) {}
  };

  public: EnumChar(Arg oArg)
    : m_pwch(oArg.m_pwch),
      m_pwchEnd(oArg.m_pwch + oArg.m_cwch) {
  }

  public: bool AtEnd() const { return m_pwch >= m_pwchEnd; }
  public: char16 Get() const { DCHECK(!AtEnd()); return *m_pwch; }
  public: void Next() { DCHECK(!AtEnd()); m_pwch++; }
};

/// <summary>
/// Inserts pwchWith after matched text then delete matched text and
/// set matched text as replaced text.
/// </summary>
class BufferOutputStream {
  private: size_t m_cwch;
  private: int m_cwchTotal;
  private: text::Range range_;
  private: text::Range* m_pMatchedRange;
  private: char16 m_rgwch[80];

  public: BufferOutputStream(text::Range* pMatchedRange)
      : m_cwch(0u), m_cwchTotal(0), range_(*pMatchedRange),
        m_pMatchedRange(pMatchedRange) {
    range_.Collapse(Collapse_End);
  }

  public: ~BufferOutputStream() {
    flush();
    // Delete matched string
    m_pMatchedRange->SetText(base::string16());
    m_pMatchedRange->MoveEnd(Unit_Char, m_cwchTotal);
  }

  // [G]
  private: void flush() {
    m_cwchTotal += m_cwch;
    range_.SetText(base::string16(m_rgwch, static_cast<size_t>(m_cwch)));
    range_.Collapse(Collapse_End);
    m_cwch = 0;
  }

  // [W]
  public: void Write(char16 wch) {
    DCHECK_LE(m_cwch, arraysize(m_rgwch));

    if (m_cwch == arraysize(m_rgwch)) {
      flush();
      DCHECK(!m_cwch);
    }
    m_rgwch[m_cwch] = wch;
    ++m_cwch;
  }

  public: void Write(const char16* pwch, int cwch) {
    foreach (EnumChar, oEnum, EnumChar::Arg(pwch, cwch)) {
      Write(oEnum.Get());
    }
  }

  public: void Write(text::Range* pRange) {
    foreach (text::Buffer::EnumChar, oEnum, pRange) {
      Write(oEnum.Get());
    }
  }
};

/// <summary>
/// Replaces matched test with specified replacement string.
/// Meta character in replacement string:
/// <list>
/// <item>
/// <term><c>$0, $1, ..., $n</c></term>
/// <description>Replace with captured string</description>
/// </item>
/// <item>
/// <term><c>$&amp;</c></term>
/// <description>Replace with the last matched string</description>
/// </item>
/// <item>
/// <term><c>${</c><i>name</i><c>}</c></term>
/// <description>Replace with named captuer</description>
/// </item>
/// <item>
/// <term><c>\</c><i>C</i></term>
/// <description>Escape next character</description>
/// </item>
/// <item>
/// <term><c>\a</c></term>
/// <description>Replace with U+0007</description>
/// </item>
/// <item>
/// <term><c>\b</c></term>
/// <description>Replace with U+0008</description>
/// </item>
/// <item>
/// <term><c>\c</c><i>C</i></term>
/// <description>Replace with U+0007</description>
/// </item>
/// <item>
/// <term><c>\e</c></term>
/// <description>Replace with U+001B</description>
/// </item>
/// <item>
/// <term><c>\f</c></term>
/// <description>Replace with U+000C</description>
/// </item>
/// <item>
/// <term><c>\n</c></term>
/// <description>Replace with U+000A</description>
/// </item>
/// <item>
/// <term><c>\r</c></term>
/// <description>Replace with U+000D</description>
/// </item>
/// <item>
/// <term><c>\t</c></term>
/// <description>Replace with U+0009</description>
/// </item>
/// <item>
/// <term><c>\u</c><i>UUUU</i></term>
/// <description>Replace with Unicode character</description>
/// </item>
/// <item>
/// <term><c>\x</c><i>XX</i></term>
/// <description>Replace with Unicode character</description>
/// </item>
/// <item>
/// <term><c>\x{</c><i>XXXX</i><c>}</c></term>
/// <description>Replace with Unicode character</description>
/// </item>
/// </list>
/// </summary>
/// <param name="pwchWith">Replacement string</param>
/// <param name="cwchWith">Number of characters in replacement string</param>
/// <param name="fMetaChar">True if enabling meta character</param>
void RegexMatcher::Replace(const base::string16& with, bool fMetaChar) {
  auto const pRange = GetMatched(0);
  if (!pRange) {
    // This matcher isn't matched.
    return;
  }

  if (!fMetaChar) {
    pRange->SetText(with);
    return;
  }

  class Util {
    public: static bool IsDigit(char16 wch, int* out_iDigit) {
      if (wch >= '0' && wch <= '9') {
        *out_iDigit = wch - '0';
        return true;
      }
      return false;
    }

    public: static bool IsNameFirst(char16 wch) {
      if (wch >= 'A' && wch <= 'Z') return true;
      if (wch >= 'a' && wch <= 'a') return true;
      if ('.' == wch) return true;
      if ('-' == wch) return true;
      if ('_' == wch) return true;
      return false;
    }

    public: static bool IsXDigit(char16 wch, int* out_iDigit) {
      if (wch >= '0' && wch <= '9')
          *out_iDigit = wch - '0';
      else if (wch >= 'A' && wch <=' F')
          *out_iDigit = wch - 'A' + 10;
      else if (wch >= 'a' && wch <=' f')
          *out_iDigit = wch - 'a' + 10;
      else
          return false;
      return true;
    }
  };

  enum State {
    State_Normal,

    State_Capture, // \<digit>+
    State_Dollar,
    State_Escape,
    State_Escape_0, // \0oo
    State_Escape_c, // \cC
    State_Escape_u, // \uXXXX
    State_Escape_x, // \xXX
    State_Escape_x4, // \x{XXXX}
    State_Ref, // ${name}
  } eState = State_Normal;

  int cChars = 0;
  int iAcc = 0;

  BufferOutputStream oStream(pRange);

  auto const pwchEnd = with.data() + with.length();
  for (auto pwch = with.data(); pwch < pwchEnd; pwch++) {
    auto const wch = *pwch;

    tryAgain:
      switch (eState) {
        case State_Normal:
          switch (wch) {
            case '$':
              if (cChars > 0) {
                oStream.Write(pwch - cChars, cChars);
                cChars = 0;
              }
              eState = State_Dollar;
              break;

            case '\\':
              if (cChars > 0) {
                oStream.Write(pwch - cChars, cChars);
                cChars = 0;
              }
              eState = State_Escape;
              break;

            default:
              ++cChars;
              break;
          }
          break;

        case State_Capture:
          if (wch >= '0' && wch <= '9') {
            iAcc *= 10;
            iAcc += wch - '0';
          } else {
            oStream.Write(GetMatched(iAcc));
            eState = State_Normal;
            goto tryAgain;
          }
          break;

        case State_Dollar:
          DCHECK(!cChars);
          switch (wch) {
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
              iAcc = wch - '0';
              eState = State_Capture;
              break;

            case '&':
              oStream.Write(GetMatched(0));
              eState = State_Normal;
              break;

            case 0x7B:
              eState = State_Ref;
              break;

            default:
              // Following Perl notations are not special meaning.
              // $$
              // $+
              // $_ match-target
              // $` match-before
              // $' match-after
              ++cChars;
              eState = State_Normal;
              break;
          }
          break;

        case State_Escape:
          DCHECK(!cChars);
          eState = State_Normal;
          switch (wch) {
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7':
              cChars = 1;
              iAcc = wch - '0';
              eState = State_Escape_0;
              break;

            case 'a':
              oStream.Write(0x07);
              break;

            case 'b':
              oStream.Write(0x08);
              break;

            case 'c':
              eState = State_Escape_c;
              break;

            case 'e':
              oStream.Write(0x1B);
              break;

            case 'f':
              oStream.Write(0x0C);
              break;

            case 'n':
              oStream.Write(0x0A);
              break;

            case 'r':
              oStream.Write(0x0D);
              break;

            case 't':
              oStream.Write(0x09);
              break;

            case 'u':
              cChars = 0;
              iAcc = 0;
              eState = State_Escape_u;
              break;

            case 'v':
              oStream.Write(0x0B);
              break;

            case 'x':
              cChars = 0;
              iAcc = 0;
              eState = State_Escape_u;
              break;

            case '\\':
              cChars = 1;
              break;

            default:
              // Insert escaped character instead of signal error
              cChars = 1;
              break;
          }
          break;

        case State_Escape_0:
          if (wch >= '0' && wch <= '7') {
            iAcc *= 8;
            iAcc |= wch - '0';
            ++cChars;
          } else {
            --pwch;
            cChars = 3;
          }

          if (3 == cChars) {
            oStream.Write(static_cast<char16>(iAcc));
            cChars = 0;
            eState = State_Normal;
          }
          break;

        case State_Escape_c:
          if (wch >= '@' && wch <= 0x5F)
            oStream.Write(static_cast<char16>(wch - '@'));
          else if (wch >= 'a' && wch <= 'z')
            oStream.Write(static_cast<char16>(wch - 0x60));
          else
           // ignore invalid replacement
          eState = State_Normal;
          break;

        case State_Escape_u: {
          int iDigit;
          if (Util::IsXDigit(wch, &iDigit)) {
            ++cChars;
            iAcc *= 16;
            iAcc |= iDigit;

            if (4 == cChars)
            {
                oStream.Write(static_cast<char16>(iAcc));
                cChars = 0;
            }
          } else {
            // ignore invalid replacement
            eState = State_Normal;
          }
          break;
        }

        case State_Escape_x:
          if (Util::IsXDigit(wch, &iAcc)) {
            iAcc = wch - '0';
            cChars = 3;
            eState = State_Escape_u;
          } else if (0x7B == wch) {
            DCHECK(!cChars);
            DCHECK(!iAcc);
            eState = State_Escape_x4;
          } else {
            // ignore invalid \xXX
            eState = State_Normal;
          }
          break;

        case State_Escape_x4: {
          int iDigit;
          if (4 == cChars) {
            if (0x7D == wch)
              oStream.Write(static_cast<char16>(iAcc));
            cChars = 0;
            eState = State_Normal;
          } else if (Util::IsXDigit(wch, &iDigit)) {
            iAcc *= 16;
            iAcc |= iDigit;
            ++cChars;
          } else {
            // ignore invalid \uXXXX
            cChars = 0;
            eState = State_Normal;
          }
          break;
      }

        case State_Ref:
          if (0x7D == wch) {
            base::string16 name(pwch - cChars, static_cast<size_t>(cChars));
            if (text::Range* pRange = GetMatched(name))
                oStream.Write(pRange);
            cChars = 0;
            eState = State_Normal;
          } else {
            ++cChars;
          }
          break;

        default:
          NOTREACHED();
      }
  }

  switch (eState) {
    case State_Normal:
      oStream.Write(pwchEnd - cChars, cChars);
      break;

    case State_Capture:
      if (auto const range = GetMatched(iAcc))
        oStream.Write(GetMatched(iAcc));
      break;
  }
}

// Continue to find match from start of document or end of document if
// this match is applied for whole document.
bool RegexMatcher::WrapMatch() {
  if (!regex_ || !match_context_ || search_params_.IsInSelection())
    return false;

  auto const pRange = match_context_->GetRange();

  if (search_params_.IsBackward())
    pRange->SetEnd(pRange->GetBuffer()->GetEnd());
  else
    pRange->SetStart(pRange->GetBuffer()->GetStart());

  return FirstMatch();
}

}  // namespace text
