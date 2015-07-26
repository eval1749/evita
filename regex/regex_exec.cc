#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// Regex - Byte Code Regex Engine
// regex/regex_exec.cpp
//
// Copyright (C) 1996-2008 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (eval1749@gmail.com)
//
// @(#)$Id: //proj/evedit2/mainline/regex/regex_exec.cpp#15 $
//
#define DEBUG_EXEC 0
#include "regex/regex.h"
#include "./regex_bytecode.h"
#include "./regex_scanner.h"
#include "./regex_util.h"

#if DEBUG_EXEC
  #define RE_DEBUG_PRINTF StdOutPrintf
#else
  #define RE_DEBUG_PRINTF __noop
#endif // DEBUG_EXEC

namespace Regex {

namespace RegexPrivate {

void StdOutPrintf(const char* pszFormat, ...) {
  va_list args;

  char szBuf[1024];
  va_start(args, pszFormat);
  ::wvsprintfA(szBuf, pszFormat, args);
  va_end(args);

  if (::IsDebuggerPresent()) {
    ::OutputDebugStringA(szBuf);
  } else {
    DWORD cbWritten;
    ::WriteFile(
      ::GetStdHandle(STD_OUTPUT_HANDLE),
      szBuf,
      ::lstrlenA(szBuf),
      &cbWritten,
      0);
  }
}

/// <remark>
/// Control opcode for control stack.
/// </remark>
enum Control {
  #define DefControl(mp_name, mp_operand) \
    Control_ ## mp_name,

  #include "./regex_bytecode.inc"

  Control_Limit,
};

/// <remark>
/// Represents a capture
/// </remark>
struct Capture {
  Posn m_lStart;
  Posn m_lEnd;

  Posn GetEnd() const { return m_lEnd; }
  Posn GetStart() const { return m_lStart; }
  bool IsBound() const { return m_lStart >= 0; }
  void MakeUnbound() { m_lStart = m_lEnd = -1; }

  void SetRange(Posn lStart, Posn lEnd) {
    m_lStart = lStart;
    m_lEnd = lEnd;
  }
};

class PosnStack {
  private: int count_;
  private: Posn* elements_;
  private: int capacity_;

  public: PosnStack(int const capacity)
    : capacity_(capacity),
      count_(0),
      elements_(new Posn[capacity]) {}

  public: ~PosnStack() { delete elements_; }

  public: Posn& operator [](int const index) {
    ASSERT(index >= 0);
    ASSERT(index < capacity_);
    return elements_[index];
  }

  public: Posn operator [](int const index) const {
    ASSERT(index >= 0);
    ASSERT(index < capacity_);
    return elements_[index];
  }

  public: int count() const { return count_; }

  public: Posn& top(int const nth) {
    ASSERT(nth >= 0);
    ASSERT(count_ - nth - 1 >= 0);
    return elements_[count_ - nth - 1];
  }

  public: Posn top(int const nth) const {
    ASSERT(nth >= 0);
    ASSERT(count_ - nth - 1 >= 0);
    return elements_[count_ - nth - 1];
  }

  public: void set_count(int count) {
    ASSERT(count >= 0);
    ASSERT(count <= capacity_);
    count_ = count;
  }

  public: void EnsureCapacity(int const size) {
    if (count_ + size < capacity_) {
      return;
    }
    auto const old_elements = elements_;
    capacity_ = (capacity_ * 3)  /2;
    elements_ = new Posn[capacity_];
    ::CopyMemory(elements_, old_elements, sizeof(Posn) * count_);
    delete [] old_elements;
  }

  public: Posn Pop() {
    ASSERT(count_ > 0);
    --count_;
    return elements_[count_];
  }

  public: void Push(Posn const datum) {
    ASSERT(count_ + 1 <= capacity_);
    elements_[count_] = datum;
    ++count_;
  }
};

/// <remark>
/// Represents regex byte code interpreter.
/// </remark>
class Engine : public Regex::SourceInfo {
  enum Limits {
    ControlStackSize = 100,
    ValueStackSize = 30,
  };

  protected: bool m_fBackward;

  /// <summary>
  /// Limit source position. Used for detecting super-linear situation.
  /// </summary>
  protected: Posn m_lLoopLimitPosn;
  protected: Posn m_lMatchEnd;
  protected: Count m_lMinLen;
  protected: Posn m_lPosn;
  protected: Posn m_lScanStop;
  protected: int m_nCxp;
  protected: int m_nPc;
  protected: IMatchContext* m_pIContext;
  protected: const int* m_prgnCode;
  protected: PosnStack control_stack_;
  protected: PosnStack value_stack_;
  protected: const Scanner* m_pScanner;

  // ctor
  public: Engine(
    IMatchContext* pIContext,
    const int* prgnCode,
    const Scanner* pScanner,
    Count lMinLen,
    bool fBackward,
    Posn lMatchEnd)
    : m_fBackward(fBackward),
      m_nCxp(0),
      m_nPc(0),
      m_lMatchEnd(lMatchEnd),
      m_lMinLen(lMinLen),
      m_lPosn(lMatchEnd),
      m_pIContext(pIContext),
      m_prgnCode(prgnCode),
      control_stack_(ControlStackSize),
      value_stack_(ValueStackSize),
      m_pScanner(pScanner) {
    m_pIContext->GetInfo(this);
  }

  protected: Engine() 
    : control_stack_(ControlStackSize),
      value_stack_(ValueStackSize) {}

  public: ~Engine() {}

  // [C]
  private: bool charEqCi(char16 a, char16 b) const {
    if (a == b) return true;
    a = m_pIContext->CharUpcase(a);
    b = m_pIContext->CharUpcase(b);
    return a == b;
  }

  private: static bool charEqCs(char16 a, char16 b) { return a == b; }

  private: static bool charSetEq(char16 a, const StringOperand* s) {
    foreach (StringOperand::Enum, oEnum, s) {
      if (oEnum.Get() == a) return true;
    }
    return false;
  }

  private: Posn cpop() {
    return control_stack_.Pop();
  }

  private: void cpush(Control eCode) {
    control_stack_.EnsureCapacity(1);
    control_stack_.Push(eCode);
  }

  private: void cpush(Control eCode, Posn a) {
    control_stack_.EnsureCapacity(2);
    control_stack_.Push(a);
    control_stack_.Push(eCode);
  }

  private: void cpush(Control eCode, Posn a, Posn b) {
    control_stack_.EnsureCapacity(3);
    control_stack_.Push(b);
    control_stack_.Push(a);
    control_stack_.Push(eCode);
  }

  private: void cpush(Control eCode, Posn a, Posn b, Posn c) {
    control_stack_.EnsureCapacity(4);
    control_stack_.Push(c);
    control_stack_.Push(b);
    control_stack_.Push(a);
    control_stack_.Push(eCode);
  }

  // [D]
  private: bool dispatch();

  // [E]
  private: bool equalCi(
      Posn lStart1,
      Posn lEnd1,
      Posn lStart2,
      Posn lEnd2) {
    unless (lEnd1 - lStart1 == lEnd2 - lStart2) return false;

    auto lPosn2 = lStart2;
    for (Posn lPosn1 = lStart1; lPosn1 < lEnd1; lPosn1++) {
      char16 wch1 = m_pIContext->GetChar(lPosn1);
      char16 wch2 = m_pIContext->GetChar(lPosn2);
      unless (charEqCi(wch1, wch2)) return false;
      lPosn2 += 1;
    }
    return true;
  }

  private: bool equalCs(
      Posn lStart1,
      Posn lEnd1,
      Posn lStart2,
      Posn lEnd2) const {
    unless (lEnd1 - lStart1 == lEnd2 - lStart2) return false;

    auto lPosn2 = lStart2;
    for (Posn lPosn1 = lStart1; lPosn1 < lEnd1; lPosn1++)
    {
      char16 wch1 = m_pIContext->GetChar(lPosn1);
      char16 wch2 = m_pIContext->GetChar(lPosn2);
      unless (charEqCs(wch1, wch2)) return false;
      lPosn2 += 1;
    }
    return true;
  }

  public: bool Execute();
  protected: bool execute() { return execute(m_lPosn); }
  protected: bool execute(Posn p) { return execute(p, p); }
  protected: bool execute(Posn, Posn);
  private: bool execute1();

  // [F]
  private: int fetchCapture(int k) const {
    return fetchInt(k);
  }

  private: Op fetchOp() const { return static_cast<Op>(fetchPosn(0)); }

  private: char16 fetchChar(int k) const {
    return static_cast<char16>(fetchPosn(k));
  }

  private: Count fetchCount(int k) const {
    return fetchPosn(k);
  }

  private: int fetchInt(int k) const {
    return static_cast<int>(fetchPosn(k));
  }

  private: Posn fetchPosn(int k) const {
    return m_prgnCode[m_nPc+k];
  }

  private: int fetchPc(int k) const {
    return static_cast<int>(fetchPosn(k));
  }

  private: const StringOperand* fetchString(int k) const {
    auto const ofs = fetchCount(k);
    return reinterpret_cast<StringOperand*>(
      reinterpret_cast<Int>(m_prgnCode) + ofs);
  }

  private: char16 getChar() const { return m_pIContext->GetChar(m_lPosn); }

  // [I]
  private: bool isBackward() const { return m_fBackward; }

  /// <summary>Predicate for ASCII word boundary.
  /// <list>
  /// <item>
  /// <term>+|foo</term>
  /// <description>
  /// Character before position isn't word character.
  /// </description>
  /// </item>
  /// <item>
  /// <term>foo|+</term>
  /// <description>
  /// Character at position isn't word character.
  /// </description>
  /// </item>
  /// </list>
  /// </summary>
  /// <returns>True if current position is ASCII word boundary</returns>
  private: bool isAsciiWordBoundary() const {
    if (m_lPosn == m_lStart) {
      return true;
    }

    if (m_lPosn == m_lEnd) {
      return true;
    }

    {
      auto const wch = m_pIContext->GetChar(m_lPosn - 1);
      if (!m_pIContext->IsAsciiWordChar(wch)) {
        return true;
      }
    }

    {
      auto const wch = m_pIContext->GetChar(m_lPosn);
      if (!m_pIContext->IsAsciiWordChar(wch)) {
        return true;
      }
    }

    return false;
  }

  private: bool isCharEq_Ci() const {
    return charEqCi(getChar(), fetchChar(1));
  }

  private: bool isCharEq_Cs() const { return getChar() == fetchChar(1); }

  private: bool isCharSetEq() const {
    return charSetEq(getChar(), fetchString(1));
  }

  private: bool isAsciiDigitCharEq() const {
    return m_pIContext->IsAsciiDigitChar(getChar());
  }

  private: bool isAsciiSpaceCharEq() const {
    return m_pIContext->IsAsciiSpaceChar(getChar());
  }

  private: bool isAsciiWordCharEq() const {
    return m_pIContext->IsAsciiWordChar(getChar());
  }

  /// <summary>
  /// Normal dollar($) or "\Z" match
  /// <list>
  /// <item><description>
  /// end of string</description></item>
  /// <item><description>
  /// before string-ending newline</description></item>
  /// </list>
  /// </summary>
  private: bool isEndOfLine() const {
    if (m_lPosn == m_lEnd) return true;
    return m_lPosn == m_lEnd - 1 && Newline == m_pIContext->GetChar(m_lPosn);
  }

  private: bool isRangeEq_Ci() const {
    auto const a = fetchChar(1);
    auto const b = getChar();
    auto const c = fetchChar(2);
    return rangeEqCi(a, b, c);
  }

  private: bool isRangeEq_Cs() const {
    auto const a = fetchChar(1);
    auto const b = getChar();
    auto const c = fetchChar(2);
    return rangeEqCs(a, b, c);
  }

  /// <summary>Predicate for Unicode word boundary.
  /// <list>
  /// <item>
  /// <term>+|foo</term>
  /// <description>
  /// Character before position isn't word character.
  /// </description>
  /// </item>
  /// <item>
  /// <term>foo|+</term>
  /// <description>
  /// Character at position isn't word character.
  /// </description>
  /// </item>
  /// </list>
  /// </summary>
  /// <returns>True if current position is Unicode word boundary</returns>
  private: bool isUnicodeWordBoundary() const {
    if (m_lPosn == m_lStart) {
      return true;
    }

    if (m_lPosn == m_lEnd) {
      return true;
    }

    {
      auto const wch = m_pIContext->GetChar(m_lPosn - 1);
      if (!m_pIContext->IsUnicodeWordChar(wch)) {
        return true;
      }
    }

    {
      auto const wch = m_pIContext->GetChar(m_lPosn);
      if (!m_pIContext->IsUnicodeWordChar(wch)) {
        return true;
      }
    }

    return false;
  }

  private: bool isUnicodeDigitCharEq() const {
    return m_pIContext->IsUnicodeDigitChar(getChar());
  }

  private: bool isUnicodeSpaceCharEq() const {
    return m_pIContext->IsUnicodeSpaceChar(getChar());
  }

  private: bool isUnicodeWordCharEq() const {
    return m_pIContext->IsUnicodeWordChar(getChar());
  }

  #define defun_ne(mp_name) \
    private: bool is ## mp_name ## Ne() const { \
      return !is ## mp_name ## Eq(); \
    }

  #define defun_ne_cs(mp_name) \
    private: bool is ## mp_name ## Ne_Ci() const { \
      return !is ## mp_name ## Eq_Ci(); \
    } \
    private: bool is ## mp_name ## Ne_Cs() const { \
      return !is ## mp_name ## Eq_Cs(); \
    }

  defun_ne_cs(Char)
  defun_ne_cs(Range)

  defun_ne(AsciiDigitChar)
  defun_ne(AsciiSpaceChar)
  defun_ne(AsciiWordChar)

  defun_ne(CharSet)

  defun_ne(UnicodeDigitChar)
  defun_ne(UnicodeSpaceChar)
  defun_ne(UnicodeWordChar)

  // [M]
  private: void makeCapturesUnbound() {
    m_pIContext->ResetCaptures();
  }

  // [R]
  private: bool rangeEqCi(char16 a, char16 b, char16 c) const {
    auto const u = m_pIContext->CharUpcase(b);
    return a <= u && u <= c;
  }

  private: static bool rangeEqCs(char16 a, char16 b, char16 c) {
    return a <= b && b <= c;
  }

  // [S]
  private: bool stringEqCi(
      Posn lStart,
      Posn lEnd,
      const StringOperand* pString) const {
    if (lEnd - lStart != pString->GetLength()) {
      return false;
    }

    for (Posn lPosn = lStart; lPosn < lEnd; lPosn++) {
      auto const fEq = charEqCi(
        m_pIContext->GetChar(lPosn),
        pString->Get(lPosn - lStart));
      unless (fEq) return false;
    }
    return true;
  }

  private: bool stringEqCs(
      Posn lStart,
      Posn lEnd,
      const StringOperand* pString) const {
    if (lEnd - lStart != pString->GetLength()) return false;
    for (Posn lPosn = lStart; lPosn < lEnd; lPosn++) {
      auto const fEq = charEqCs(
        m_pIContext->GetChar(lPosn),
        pString->Get(lPosn - lStart));
      unless (fEq) return false;
    }
    return true;
  }

  // [V]
  private: Posn vpop() {
    return value_stack_.Pop();
  }

  private: void vpush(Posn const value) {
    value_stack_.EnsureCapacity(1);
    value_stack_.Push(value);
  }

  #if DEBUG_EXEC
    char16 debugGetChar(Posn lPosn) const {
      if (lPosn < m_lStart) return '_';
      if (lPosn >= m_lEnd) return '_';
      auto const wch = static_cast<char16>(
          m_pIContext->GetChar(lPosn) & 0xFF);
      if (wch < 0x20) return '.';
      if (wch > 0x7E) return '.';
      return wch;
    }

    void printControl() const;
  #endif // DEBUG_EXEC
};

#if DEBUG_EXEC

enum OpFormat {
  OpFormat_none,

  OpFormat_c,
  OpFormat_cc,
  OpFormat_l,
  OpFormat_ln,
  OpFormat_n,
  OpFormat_nc,
  OpFormat_ncc,
  OpFormat_nn,
  OpFormat_ns,
  OpFormat_s,

  // Control instruction
  OpFormat_nth_start_end,
  OpFormat_nextPc_posn,
  OpFormat_int,
  OpFormat_nth,
  OpFormat_posn,
  OpFormat_nextPc_posn_minPosn,
  OpFormat_nextPc_posn_maxPosn,
  OpFormat_cxp_vsp,
};

struct OpDesc {
  OpFormat m_eFormat;
  const char* m_pszMnemonic;
};

static const OpDesc k_rgoOpDesc[Op_Limit + Control_Limit] = {
  #define DefByteCode(mp_mnemonic, mp_operands) \
    { OpFormat_ ## mp_operands, #mp_mnemonic },

  #define DefControl(mp_mnemonic, mp_operands) \
    { OpFormat_ ## mp_operands, #mp_mnemonic },

  #include "regex_bytecode.inc"
};

static void printCharOperand(int iChar) {
  if (iChar >= 0x20 && iChar <= 0x7E) {
     StdOutPrintf("'%c'", iChar);
  } else {
    StdOutPrintf("'\\u%04X'", iChar);
  }
}

/// <summary>
/// Prints one instruction
/// </summary>
/// <param name="prgnCode">Start of regex bytecode sequence</param>
/// <param name="nPc">Instruction to print</param>
static int printOp(const int* prgnCode, int nPc) {
  auto const eOp = static_cast<Op>(prgnCode[nPc]);

  auto const pDesc = &k_rgoOpDesc[eOp];

  StdOutPrintf("L%03d %s", nPc, pDesc->m_pszMnemonic);

  nPc += 1;

  switch (pDesc->m_eFormat) {
    case OpFormat_c:
      StdOutPrintf(" ");
      printCharOperand(prgnCode[nPc]);
      nPc += 1;
      break;

    case OpFormat_cc:
      StdOutPrintf(" ");
      printCharOperand(prgnCode[nPc]);
      StdOutPrintf(" ");
      printCharOperand(prgnCode[nPc+1]);
      nPc += 2;
      break;

    case OpFormat_l:
      StdOutPrintf(" L%03d", prgnCode[nPc]);
      nPc += 1;
      break;

    case OpFormat_ln:
      StdOutPrintf(" L%03d %d", prgnCode[nPc], prgnCode[nPc+1]);
      nPc += 2;
      break;

    case OpFormat_n:
      StdOutPrintf(" %d", prgnCode[nPc]);
      nPc += 1;
      break;

    case OpFormat_nc:
      StdOutPrintf(" %d ", prgnCode[nPc]);
      printCharOperand(prgnCode[nPc+1]);
      nPc += 2;
      break;

    case OpFormat_ncc:
      StdOutPrintf(" %d ", prgnCode[nPc]);
      printCharOperand(prgnCode[nPc+1]);
      StdOutPrintf(" ");
      printCharOperand(prgnCode[nPc+2]);
      nPc += 3;
      break;

    case OpFormat_nn:
      StdOutPrintf(" %d %d", prgnCode[nPc], prgnCode[nPc+1]);
      nPc += 2;
      break;

    case OpFormat_none:
      break;

    case OpFormat_ns: {
      auto const pString = reinterpret_cast<const StringOperand*>(
          reinterpret_cast<Int>(prgnCode) + prgnCode[nPc+1]);

      StdOutPrintf(" %d \"%ls\"", prgnCode[nPc], pString->Get());
      nPc += 2;
      break;
    }

    case OpFormat_s:
    {
      auto const pString = reinterpret_cast<const StringOperand*>(
          reinterpret_cast<Int>(prgnCode) + prgnCode[nPc]);

      StdOutPrintf(" \"%ls\"", pString->Get());
      nPc += 1;
      break;
    }
 }

  return nPc;
}

static const char* const k_rgpszMethod[] = {
  "None",

  "CharCiBackward",
  "CharCiForward",

  "StringCiBackward",
  "StringCiForward",

  "CharCsBackward",
  "CharCsForward",

  "StringCsBackward",
  "StringCsForward",

  "FullBackward",
  "FullForward",

  "ZeroWidth", // for "^", "$", and so on.
};

/// <summary>
/// Prints description of this regex object.
/// </summary>
void RegexObj::Describe() const {
  StdOutPrintf("Regex object @ %p\n", this);
  StdOutPrintf(" scan method = %s\n",
      k_rgpszMethod[GetScanner()->GetMethod()]);
  StdOutPrintf(" max capture = %d\n", m_nMaxCapture);
  StdOutPrintf(" min length = %d\n", m_nMinLen);
  StdOutPrintf(" options = 0x%x\n", m_rgfOption);
  StdOutPrintf("\n");

  #if 0
  {
    char* pszTitle = " named captures:\n";
    char* pszFooter = "";
    foreach (EnumCapture, oEnum, this) {
      Capture* pCapture = oEnum.Get();
      if (!pCapture->m_pwszName) {
        continue;
      }

      StdOutPrintf("%s [%d] '%ls'\n",
        pszTitle,
        pCapture->m_iNth,
        pCapture->m_pwszName);
      pszTitle= "";
      pszFooter = "\n";
    }
    StdOutPrintf(pszFooter);
  }
  #endif

  StdOutPrintf(" bytecode:\n");
  int nPc = 0;
  const int* prgnCode = GetCodeStart();
  for (;;) {
    Op eOp = static_cast<Op>(prgnCode[nPc]);
    if (Op_End == eOp) {
      break;
    }

    StdOutPrintf(" ");
    nPc = printOp(prgnCode, nPc);
    StdOutPrintf("\n");
  }
}
#elif defined(_DEBUG)
void RegexObj::Describe() const {
}
#endif // DEBUG_EXEC

#if DEBUG_EXEC
void Engine::printControl() const {
  auto const opcode = control_stack_.top(0);
  auto const p = &k_rgoOpDesc[opcode + Op_Limit];

  StdOutPrintf("%s", p->m_pszMnemonic);

  switch (p->m_eFormat) {
    case OpFormat_nth_start_end:
      StdOutPrintf(" nth=%d start=%d %d",
        control_stack_.top(1),
        control_stack_.top(2),
        control_stack_.top(3));
      break;

    case OpFormat_nextPc_posn:
      StdOutPrintf(" nextPc=%d posn=%d",
        control_stack_.top(1),
        control_stack_.top(2));
      break;

    case OpFormat_int:
      StdOutPrintf(" int=%d", control_stack_.top(1));
      break;

    case OpFormat_none:
      StdOutPrintf("");
      break;

    case OpFormat_nth:
      StdOutPrintf(" nth=%d", control_stack_.top(1));
      break;

    case OpFormat_posn:
      StdOutPrintf(" posn=%d", control_stack_.top(1));
      break;

    case OpFormat_nextPc_posn_minPosn:
      StdOutPrintf(" nextPc=%d posn=%d minPosn=%d",
        control_stack_.top(1),
        control_stack_.top(2),
        control_stack_.top(3));
      break;

    case OpFormat_nextPc_posn_maxPosn:
      StdOutPrintf(" nextPc=%d posn=%d maxPosn=%d",
        control_stack_.top(1),
        control_stack_.top(2),
        control_stack_.top(3));
      break;

    case OpFormat_cxp_vsp:
      StdOutPrintf(" cxp=%d vsp=%d",
        control_stack_.top(1),
        control_stack_.top(2));
      break;

    default:
      NOTREACHED();
  }
}
#endif // DEBUG_EXEC

/// <summary>
/// Opcode dispatcher
/// </summary>
/// <returns>
/// True if engine executes Op_Success, false otherwise.
/// </returns>
bool Engine::dispatch() {
  for (;;) {
    #if DEBUG_EXEC
    {
      StdOutPrintf("<");
      for (
          Posn lPosn = m_lPosn - 7;
          lPosn < m_lPosn;
          lPosn += 1) {
        StdOutPrintf("%c", debugGetChar(lPosn));
      }

      StdOutPrintf(">%c<", debugGetChar(m_lPosn));

      for (
          Posn lPosn = m_lPosn + 1;
          lPosn < m_lPosn + 7;
          lPosn += 1) {
        StdOutPrintf("%c", debugGetChar(lPosn));
      }
      StdOutPrintf("> ");

      StdOutPrintf("csp=%03d vsp=%03d pos=%03d|%c| ",
          control_stack_.count(),
          value_stack_.count(),
          m_lPosn,
          debugGetChar(m_lPosn));
      printOp(m_prgnCode, m_nPc);
      StdOutPrintf("\n");
    }
    #endif // DEBUG_EXEC

    switch (fetchOp()) {
      // [A]
      case Op_AfterNewline:
        // "(?m:^)" = matches
        // at start of string, or
        // after any newline
        if (m_lPosn == m_lStart
            || Newline == m_pIContext->GetChar(m_lPosn - 1)) {
          m_nPc += 1;
          break;
        }
        return false;

      case Op_Any_B: // Matches any character. ".", "\p{ANY}"
        m_lPosn -= 1;
        unless (m_lPosn >= m_lStart) return false;
        m_nPc += 1;
        break;

      case Op_Any_F:
        unless (m_lPosn < m_lEnd) return false;
        m_lPosn += 1;
        m_nPc += 1;
        break;

      case Op_AsciiBoundary: // Matches word boundary. "\b"
        unless (isAsciiWordBoundary()) return false;
        m_nPc += 1;
        break;

      case Op_AsciiNotBoundary: // Matches not word boundary. ("\B")
        unless (isAsciiWordBoundary()) return false;
        m_nPc += 1;
        break;

      #define OP_IMPL_ONE_WIDTH_B(mp_size, mp_test) \
      { \
        m_lPosn -= 1; \
        if (m_lPosn >= m_lStart && mp_test()) { \
          m_nPc += mp_size; \
          break; \
        } \
        return false; \
      }

      #define OP_IMPL_ONE_WIDTH_F(mp_size, mp_test) \
      { \
        if (m_lPosn < m_lEnd && mp_test()) { \
          m_lPosn += 1; \
          m_nPc += mp_size; \
          break; \
        } \
        return false; \
      }

      #define case_Op_OneWidth_BF(mp_name, mp_size) \
        case Op_ ## mp_name ## _B: \
          OP_IMPL_ONE_WIDTH_B(mp_size, is ## mp_name); \
        case Op_ ## mp_name ## _F: \
          OP_IMPL_ONE_WIDTH_F(mp_size, is ## mp_name);

      case_Op_OneWidth_BF(AsciiDigitCharEq, 1) // \d
      case_Op_OneWidth_BF(AsciiDigitCharNe, 1) // \D

      case_Op_OneWidth_BF(AsciiSpaceCharEq, 1) // \s
      case_Op_OneWidth_BF(AsciiSpaceCharNe, 1) // \S

      case_Op_OneWidth_BF(AsciiWordCharEq, 1) // \w
      case_Op_OneWidth_BF(AsciiWordCharNe, 1) // \W

      // [B]
      case Op_BeforeNewline: // Matches before any newline. "(?m:$)"
        if (m_lPosn == m_lEnd || Newline == getChar()) {
          m_nPc += 1;
          break;
        }
        return false;

      // [C]
      case Op_Capture_B: { // nNth
        auto const nCapture = fetchCapture(1);
        auto const lEnd = vpop();

        cpush(Control_PushPosn, lEnd);

        Posn lCaptureStart;
        Posn lCaptureEnd;

        auto const fHasCapture = m_pIContext->GetCapture(
            nCapture,
            &lCaptureStart,
            &lCaptureEnd);

        if (fHasCapture) {
          cpush(
            Control_Capture,
            fetchInt(1),
            lCaptureStart,
            lCaptureEnd);
        } else {
          cpush(Control_ResetCapture, fetchInt(1));
        }

        m_pIContext->SetCapture(nCapture, m_lPosn, lEnd);

        #if DEBUG_EXEC
          StdOutPrintf("\tCapture[%d]=%d..%d <",
            fetchInt(1),
            m_lPosn,
            lEnd);
          for (auto lPosn = m_lPosn; lPosn < lEnd; lPosn++) {
            StdOutPrintf("%c", debugGetChar(lPosn));
          }
          StdOutPrintf(">\n");
        #endif // DEBUG_EXEC

        m_nPc += 2;
        break;
      }

      case Op_Capture_F: { // nNth
        auto const nCapture = fetchCapture(1);
        auto const lStart = vpop();

        cpush(Control_PushPosn, lStart);

        Posn lCaptureStart;
        Posn lCaptureEnd;
        auto const fHasCapture = m_pIContext->GetCapture(
          nCapture,
          &lCaptureStart,
          &lCaptureEnd);

        if (fHasCapture) {
          cpush(
              Control_Capture,
              fetchInt(1),
              lCaptureStart,
              lCaptureEnd);
        } else {
          cpush(Control_ResetCapture, fetchInt(1));
        }

        m_pIContext->SetCapture(nCapture, lStart, m_lPosn);

        #if DEBUG_EXEC
          StdOutPrintf("\tCapture[%d]=%d..%d<",
            fetchInt(1),
            lStart,
            m_lPosn);
          for (auto lPosn = lStart; lPosn < m_lPosn; lPosn++) {
            StdOutPrintf("%c", debugGetChar(lPosn));
          }
          StdOutPrintf(">\n");
        #endif // DEBUG_EXEC

        m_nPc += 2;
        break;
      }

      case Op_CaptureIfNot: { // lable nth
        auto const nBranchPc = fetchPc(1);
        auto const nCapture = fetchCapture(2);
        Posn lStart, lEnd;
        if (m_pIContext->GetCapture(nCapture, &lStart, &lEnd)) {
          m_nPc += 3;
        }
        else
        {
          m_nPc = nBranchPc;
        }
        break;
      }

      case Op_CaptureEq_Ci_B: { // nth
        auto const nCapture = fetchCapture(1);
        Posn lStart;
        Posn lEnd;
        if (m_pIContext->GetCapture(nCapture, &lStart, &lEnd)) {
          auto const lNextPosn = m_lPosn + (lEnd - lStart);
          if (lNextPosn >= m_lStart) {
            if (equalCi(lStart, lEnd, lNextPosn, m_lPosn)) {
              m_lPosn = lNextPosn;
              m_nPc += 2;
              break;
            }
          }
        }
        return false;
      }

      case Op_CaptureEq_Ci_F: { // nth
        Posn lStart;
        Posn lEnd;
        auto const nCapture = fetchCapture(1);
        if (m_pIContext->GetCapture(nCapture, &lStart, &lEnd)) {
          auto const lNextPosn = m_lPosn + (lEnd - lStart);
          if (lNextPosn <= m_lEnd) {
            if (equalCs(lStart, lEnd, m_lPosn, lNextPosn)) {
              m_lPosn = lNextPosn;
              m_nPc += 2;
              break;
            }
          }
        }
        return false;
      }

      case Op_CaptureEq_Cs_B: { // nth
        Posn lStart;
        Posn lEnd;
        auto const nCapture = fetchCapture(1);
        if (m_pIContext->GetCapture(nCapture, &lStart, &lEnd)) {
          auto const lNextPosn = m_lPosn + (lEnd - lStart);
          if (lNextPosn >= m_lStart) {
            if (equalCi(lStart, lEnd, lNextPosn, m_lPosn)) {
              m_lPosn = lNextPosn;
              m_nPc += 2;
              break;
            }
          }
        }
        return false;
      }

      case Op_CaptureEq_Cs_F: { // nth
        Posn lStart;
        Posn lEnd;
        auto const nCapture = fetchCapture(1);
        if (m_pIContext->GetCapture(nCapture, &lStart, &lEnd)) {
          auto const lNextPosn = m_lPosn + (lEnd - lStart);
          if (lNextPosn <= m_lEnd) {
            if (equalCs(lStart, lEnd, m_lPosn, lNextPosn)) {
              m_lPosn = lNextPosn;
              m_nPc += 2;
              break;
            }
          }
        }
        return false;
      }

      case Op_CaptureMakeUnbound: {
        int nCapture = fetchCapture(1);
        m_pIContext->ResetCapture(nCapture);
        m_nPc += 2;
        break;
      }

      case Op_CategoryEq_B:
      case Op_CategoryEq_F:
      case Op_CategoryNe_B:
      case Op_CategoryNe_F:
      case Op_CategoryLe_B:
      case Op_CategoryLe_F:
      case Op_CategoryGt_B:
      case Op_CategoryGt_F:
        NOTREACHED();

      case_Op_OneWidth_BF(CharEq_Ci, 2)
      case_Op_OneWidth_BF(CharEq_Cs, 2)
      case_Op_OneWidth_BF(CharNe_Ci, 2)
      case_Op_OneWidth_BF(CharNe_Cs, 2)

      case_Op_OneWidth_BF(CharSetEq, 2)
      case_Op_OneWidth_BF(CharSetNe, 2)

      // [E]
      case Op_EndOfLine:
        if (isEndOfLine()) {
          m_nPc += 1;
          break;
        }
        return false;

      case Op_EndOfString: // Matches end of string. "\z"
        if (m_lPosn == m_lEnd) {
          m_nPc += 1;
          break;
        }
        return false;

      // [F]
      case Op_Fail:
        return false;

      // [G]
      case Op_Go: // label
        m_nPc = fetchPosn(1);
        break;

      // [L]
      case Op_Last_B: { // minRest
        auto const nMinRest = fetchInt(1);
        auto const lNextPosn = m_lPosn + nMinRest;
        unless (lNextPosn >= m_lStart) return false;

        if (lNextPosn <= m_lLoopLimitPosn) {
          RE_DEBUG_PRINTF("We won't check after %d/%d\n",
            lNextPosn, m_lLoopLimitPosn);
          return false;
        }

        if (lNextPosn - 1 == m_lLoopLimitPosn) {
          m_lLoopLimitPosn = lNextPosn;
        }
        m_nPc += 2;
        break;
      }

      case Op_Last_F: { // minRest
        auto const nMinRest = fetchInt(1);
        auto const lNextPosn = m_lPosn - nMinRest;
        unless (lNextPosn <= m_lEnd) {
          RE_DEBUG_PRINTF("\tWe need at least %d, but %d.\n",
            nMinRest,
            lNextPosn - m_lEnd);
          return false;
        }

        if (lNextPosn >= m_lLoopLimitPosn) {
          RE_DEBUG_PRINTF("\tWe won't check after %d/%d\n",
              lNextPosn, m_lLoopLimitPosn);
          return false;
        }

        if (lNextPosn - 1 == m_lLoopLimitPosn) {
          m_lLoopLimitPosn = lNextPosn;
        }
        m_nPc += 2;
        break;
      }

      // [M]
      case Op_Max: { // label max-min
        auto const nLoopPc = fetchPosn(1);
        auto const nMaxMin = fetchInt(2);
        auto const nNextPc = m_nPc + 3;
        auto nCounter = vpop();
        cpush(Control_PushInt, nCounter);
        nCounter += 1;
        if (nCounter == nMaxMin) {
          RE_DEBUG_PRINTF("\t\t matched: counter=%d\n", nCounter);
          m_nPc = nNextPc;
        } else if (nCounter < 0) {
          RE_DEBUG_PRINTF("\t\t need more: counter=%d\n", nCounter);
          cpush(Control_PopInt);
          vpush(nCounter);
          m_nPc = nLoopPc;
        } else {
          RE_DEBUG_PRINTF("\t\t try more: counter=%d\n", nCounter);
          cpush(Control_Continue, nNextPc, m_lPosn);
          cpush(Control_PopInt);
          vpush(nCounter);
          m_nPc = nLoopPc;
        }
        break;
      }

      case Op_Min: {
        auto const nLoopPc = fetchPosn(1);
        auto const nMaxMin = fetchInt(2);
        auto const nNextPc = m_nPc + 3;
        auto nCounter = vpop();

        cpush(Control_PushInt, nCounter);
        nCounter += 1;
        if (nCounter == nMaxMin) {
          m_nPc = nNextPc;
        } else if (nCounter < 0) {
          cpush(Control_PopInt);
          vpush(nCounter);
          m_nPc = nLoopPc;
        } else {
          cpush(Control_Continue, nNextPc, m_lPosn);
          cpush(Control_PushInt, nCounter);
          m_nPc = nLoopPc;
        }
        break;
      }

      // [N]
      case Op_Nulc: { // exitPc
        auto const nExitPc = fetchPc(1);
        auto const nNextPc = m_nPc + 2;
        auto const lMarkPosn = vpop();
        cpush(Control_PushPosn, lMarkPosn);
        if (m_lPosn != lMarkPosn) {
          m_nPc = nNextPc;
        } else {
          auto const nCounter = vpop();
          cpush(Control_PushInt, nCounter);
          m_nPc = nExitPc;
        }
        break;
      }

      case Op_Null: { // exitPc
        auto const nExitPc = fetchPc(1);
        auto const nNextPc = m_nPc + 2;
        auto const lMarkPosn = vpop();
        cpush(Control_PushPosn, lMarkPosn);
        if (m_lPosn != lMarkPosn) {
          m_nPc = nNextPc;
        } else {
          m_nPc = nExitPc;
        }
        break;
      }

      case Op_Or: { // label
        auto const nLabelPc = fetchPc(1);
        cpush(Control_Continue, nLabelPc, m_lPosn);
        m_nPc += 2;
        break;
      }

      // [P]
      case Op_Posn: // Matches end of last match. "\G"
        if (m_lPosn == m_lMatchEnd) {
          m_nPc += 1;
          break;
        }
        return false;

      case Op_Push: // label
        cpush(Control_Continue, m_nPc + 2, m_lPosn);
        m_nPc = fetchPc(1);
        break;

      case Op_PushInt: // n
        cpush(Control_PopInt);
        vpush(fetchCount(1));
        m_nPc += 2;
        break;

      case Op_PushPosn:
        cpush(Control_PopPosn);
        vpush(m_lPosn);
        m_nPc += 1;
        break;

      // [R]
      case_Op_OneWidth_BF(RangeEq_Ci, 3)
      case_Op_OneWidth_BF(RangeEq_Cs, 3)
      case_Op_OneWidth_BF(RangeNe_Ci, 3)
      case_Op_OneWidth_BF(RangeNe_Cs, 3)

      case Op_RepeatAny_B: { // rest
        auto const nRest = fetchCount(1);
        auto const lNextPosn = m_lStart + nRest;
        auto const nNextPc = m_nPc + 2;
        if (lNextPosn < m_lPosn) {
          auto const lMaxPosn = m_lPosn;
          m_lPosn = lNextPosn;
          cpush(Control_Repeat_B, nNextPc, m_lPosn, lMaxPosn);
          m_nPc = nNextPc;
          break;
        }
        return false;
      }

      case Op_RepeatAny_F: { // rest
        auto const nRest = fetchCount(1);
        auto const lNextPosn = m_lEnd - nRest;
        auto const nNextPc = m_nPc + 2;
        if (lNextPosn > m_lPosn) {
          auto const lMaxPosn = m_lPosn;
          m_lPosn = lNextPosn;
          cpush(Control_Repeat_F, nNextPc, m_lPosn, lMaxPosn);
          m_nPc = nNextPc;
          break;
        }
        return false;
      }

      #define REPEAT_BACKWARD(mp_opsize, mp_cmp) \
      { \
        auto const nRest = fetchCount(1); \
        auto const lMaxPosn = m_lPosn; \
        auto const lNextPosn = m_lStart + nRest; \
        unless (lNextPosn < m_lPosn) return false; \
        while (m_lPosn > lNextPosn) { \
          m_lPosn -= 1; \
          char16 wchSource = getChar(); \
          unless (mp_cmp) break; \
        } \
        if (m_lPosn == lMaxPosn) return false; \
        { \
          auto const nNextPc = m_nPc + mp_opsize; \
          cpush(Control_Repeat_B, nNextPc, m_lPosn, lMaxPosn); \
          m_nPc = nNextPc; \
        } \
        break; \
      }

      #define REPEAT_FORWARD(mp_opsize, mp_cmp) \
      { \
        auto const nRest = fetchCount(1); \
        auto const lMinPosn = m_lPosn; \
        auto const lNextPosn = m_lEnd - nRest; \
        unless (lNextPosn > m_lPosn) return false; \
        while (m_lPosn < lNextPosn) { \
          auto const wchSource = getChar(); \
          unless (mp_cmp) break; \
          m_lPosn += 1; \
        } \
        if (m_lPosn == lMinPosn) return false; \
        { \
          auto const nNextPc = m_nPc + mp_opsize; \
          cpush(Control_Repeat_F, nNextPc, m_lPosn, lMinPosn); \
          m_nPc = nNextPc; \
        } \
        break; \
      }

      #define case_Op_RepeatChar_BF(mp_Eq, mp_eq) \
        case Op_RepeatChar ## mp_Eq ## _B: \
        { \
          auto const wchOperand = fetchChar(2); \
          REPEAT_BACKWARD(3, mp_eq(wchSource, wchOperand)) \
        } \
        case Op_RepeatChar ## mp_Eq ## _F: \
        { \
          auto const wchOperand = fetchChar(2); \
          REPEAT_FORWARD(3, mp_eq(wchSource, wchOperand)) \
        }

      #define case_Op_Repeat_CharSet_BF(mp_Eq, mp_eq) \
        case Op_RepeatCharSet ## mp_Eq ## _B: \
        { \
          const StringOperand* pOperand = fetchString(2); \
          REPEAT_BACKWARD(3, mp_eq(wchSource, pOperand)) \
        } \
        case Op_RepeatCharSet ## mp_Eq ## _F: \
        { \
          const StringOperand* pOperand = fetchString(2); \
          REPEAT_FORWARD(3, mp_eq(wchSource, pOperand)) \
        }

      #define case_Op_RepeatRange_BF(mp_Eq, mp_eq) \
        case Op_RepeatRange ## mp_Eq ## _B: \
        { \
          auto const wchMin = fetchChar(2); \
          auto const wchMax = fetchChar(3); \
          REPEAT_BACKWARD(4, mp_eq(wchMin, wchSource, wchMax)) \
        } \
        case Op_RepeatRange ## mp_Eq ## _F: \
        { \
          auto const wchMin = fetchChar(2); \
          auto const wchMax = fetchChar(3); \
          REPEAT_FORWARD(4, mp_eq(wchMin, wchSource, wchMax)) \
        }

      #define case_Op_Repeat_Unit_CBF(mp_Unit, mp_Eq, mp_eq) \
        case_Op_Repeat ## mp_Unit ## _BF(mp_Eq ## _Ci, mp_eq ## Ci) \
        case_Op_Repeat ## mp_Unit ## _BF(mp_Eq ## _Cs, mp_eq ## Cs) \

      #define case_Op_Repeat_OneWidth_BF(mp_class) \
        case Op_Repeat ## mp_class ## Eq_B: \
          REPEAT_BACKWARD(2, \
            m_pIContext->Is ## mp_class(wchSource)); \
        case Op_Repeat ## mp_class ## Ne_B: \
          REPEAT_BACKWARD(2, \
            !m_pIContext->Is ## mp_class(wchSource)); \
        case Op_Repeat ## mp_class ## Eq_F: \
          REPEAT_FORWARD(2, \
            m_pIContext->Is ## mp_class(wchSource)); \
        case Op_Repeat ## mp_class ## Ne_F: \
          REPEAT_FORWARD(2, \
            !m_pIContext->Is ## mp_class(wchSource));

      case_Op_Repeat_Unit_CBF(Char, Eq, charEq)
      case_Op_Repeat_Unit_CBF(Char, Ne, !charEq)

      case_Op_Repeat_Unit_CBF(Range, Eq, rangeEq)
      case_Op_Repeat_Unit_CBF(Range, Ne, !rangeEq)

      case_Op_Repeat_CharSet_BF(Eq, charSetEq)
      case_Op_Repeat_CharSet_BF(Ne, !charSetEq)

      case_Op_Repeat_OneWidth_BF(AsciiDigitChar)
      case_Op_Repeat_OneWidth_BF(AsciiSpaceChar)
      case_Op_Repeat_OneWidth_BF(AsciiWordChar)

      case_Op_Repeat_OneWidth_BF(UnicodeDigitChar)
      case_Op_Repeat_OneWidth_BF(UnicodeSpaceChar)
      case_Op_Repeat_OneWidth_BF(UnicodeWordChar)

      case Op_Rest_B: { // rest
        auto const nRest = fetchCount(1);
        auto const lNextPosn = m_lPosn - nRest;
        if (lNextPosn >= m_lStart && lNextPosn > m_lLoopLimitPosn) {
          m_nPc += 2;
          break;
        }
        return false;
      }

      case Op_Rest_F: { // rest
        auto const nRest = fetchCount(1);
        auto const lNextPosn = m_lPosn + nRest;
        if (lNextPosn <= m_lEnd && lNextPosn < m_lLoopLimitPosn) {
          m_nPc += 2;
          break;
        }
        return false;
      }

      case Op_RestoreCxp: {
        auto const index = m_nCxp;
        ASSERT(control_stack_[index - 1] == Control_SaveCxp);
        m_nCxp = control_stack_[index - 2];
        value_stack_.set_count(control_stack_[index - 3]);
        control_stack_.set_count(index - 4);
        m_nPc += 1;
        break;
      }

      case Op_RestorePosn:
        m_lPosn = vpop();
        cpush(Control_SavePosn, m_lPosn);
        m_nPc += 1;
        break;

      // [S]
      case Op_SaveCxp:
        cpush(Control_SaveCxp, m_nCxp, value_stack_.count());
        m_nCxp = control_stack_.count();
        m_nPc += 1;
        break;

      case Op_SavePosn:
        vpush(m_lPosn);
        m_nPc += 1;
        break;

      case Op_StartOfString: // Matches start of string. "(?-m)^": "\A"
        unless (m_lPosn == m_lStart) return false;
        m_nPc += 1;
        break;

      #define OP_IMPL_STRING_TEST_B(mp_cmp) \
      { \
        auto const pString = fetchString(1); \
        auto const lNextPosn = m_lPosn - pString->GetLength(); \
        unless (lNextPosn >= m_lScanStart) return false; \
        unless (mp_cmp(lNextPosn, m_lPosn, pString)) return false; \
        m_lPosn = lNextPosn; \
        m_nPc += 2; \
        break; \
      }

      #define OP_IMPL_STRING_TEST_F(mp_cmp) \
      { \
        auto const pString = fetchString(1); \
        auto const lNextPosn = m_lPosn + pString->GetLength(); \
        unless (lNextPosn <= m_lScanEnd) return false; \
        unless (mp_cmp(m_lPosn, lNextPosn, pString)) return false; \
        m_lPosn = lNextPosn; \
        m_nPc += 2; \
        break; \
      }

      case Op_StringEq_Ci_B:
        OP_IMPL_STRING_TEST_B(stringEqCi)

      case Op_StringEq_Ci_F:
        OP_IMPL_STRING_TEST_F(stringEqCi)

      case Op_StringEq_Cs_B:
        OP_IMPL_STRING_TEST_B(stringEqCs)

      case Op_StringEq_Cs_F:
        OP_IMPL_STRING_TEST_F(stringEqCs)

      case Op_Success:
        return true;

      // [U]
      case Op_UnicodeBoundary: // \b
        unless (isUnicodeWordBoundary()) return false;
        m_nPc += 1;
        break;

      case Op_UnicodeNotBoundary: // \B
        unless (isUnicodeWordBoundary()) return false;
        m_nPc += 1;
        break;

      case_Op_OneWidth_BF(UnicodeDigitCharEq, 1) // \d
      case_Op_OneWidth_BF(UnicodeDigitCharNe, 1) // \D

      case_Op_OneWidth_BF(UnicodeSpaceCharEq, 1) // \s
      case_Op_OneWidth_BF(UnicodeSpaceCharNe, 1) // \S

      case_Op_OneWidth_BF(UnicodeWordCharEq, 1) // \w
      case_Op_OneWidth_BF(UnicodeWordCharNe, 1) // \W

      default:
        DEBUG_PRINTF("Unknown op: 0x%x\n", fetchOp());
        return false;
    }
  }
}

class CiCompare {
  public: static bool BackwardFindChar(
      IMatchContext* pIContext,
      char16 wchLast,
      Posn* inout_lPosn,
      Posn lStop) {
    return pIContext->BackwardFindCharCi(wchLast, inout_lPosn, lStop);
  }

  public: static char16 Fold(
      IMatchContext* pIContext,
      char16 wch) {
    return pIContext->CharUpcase(wch);
  }

  public: static bool ForwardFindChar(
      IMatchContext* pIContext,
      char16 wchLast,
      Posn* inout_lPosn,
      Posn lStop) {
    return pIContext->ForwardFindCharCi(wchLast, inout_lPosn, lStop);
  }

  public: static bool StringEq(
      IMatchContext* pIContext,
      const char16* pwch,
      int cwch,
      Posn lPosn) {
    return pIContext->StringEqCi(pwch, cwch, lPosn);
  }
};

class CsCompare {
  public: static bool BackwardFindChar(
      IMatchContext* pIContext,
      char16 wchFirst,
      Posn* inout_lPosn,
      Posn lStop) {
    ASSERT(*inout_lPosn >= 0);
    return pIContext->BackwardFindCharCs(wchFirst, inout_lPosn, lStop);
  }

  public: static char16 Fold(IMatchContext*, char16 wch) {
    return wch;
  }

  public: static bool ForwardFindChar(
      IMatchContext* pIContext,
      char16 wchFirst,
      Posn* inout_lPosn,
      Posn lStop) {
    ASSERT(*inout_lPosn >= 0);
    return pIContext->ForwardFindCharCs(wchFirst, inout_lPosn, lStop);
  }

  public: static bool StringEq(
      IMatchContext* pIContext,
      const char16* pwch,
      int cwch,
      Posn lPosn) {
    ASSERT(lPosn >= 0);
    return pIContext->StringEqCs(pwch, cwch, lPosn);
  }
};

template<class Case_>
class CharScanner_ : public CharScanner {
  public: bool ScanBackward(
      IMatchContext* pIContext,
      Posn* inout_lPosn,
      Posn lScanStop) const {
    return Case_::BackwardFindChar(
        pIContext,
        getChar(),
        inout_lPosn,
        lScanStop);
  }

  public: bool ScanForward(
      IMatchContext* pIContext,
      Posn* inout_lPosn,
      Posn lScanStop) const {
    return Case_::ForwardFindChar(
        pIContext,
        getChar(),
        inout_lPosn,
        lScanStop);
  }
};

template<class Case_>
class StringScanner_ : public StringScanner {
  public: bool ScanBackward(
      IMatchContext* pIContext,
      Posn* inout_lPosn,
      Posn lScanStop) const {
    auto const wchLast = getString()[m_cwch - 1];
    auto const lLoopStop = lScanStop + m_cwch;
    auto lPosn = *inout_lPosn;
    while (lPosn >= lLoopStop) {
      auto const fFound = Case_::BackwardFindChar(
          pIContext,
          wchLast,
          &lPosn,
          lLoopStop);
      if (!fFound)
      {
        return false;
      }

      // foo
      // ^ ^=lPosn
      // = lNextPosn
      auto lNextPosn = lPosn - m_cwch;

      auto const fEqual = Case_::StringEq(
          pIContext,
          getString(),
          m_cwch,
          lNextPosn);

      if (fEqual) {
        *inout_lPosn = lPosn;
        return true;
      }

      lNextPosn -= 1;
      if (lNextPosn <= lScanStop) {
        return false;
      }

      auto const wch = Case_::Fold(pIContext, pIContext->GetChar(lNextPosn));
      lPosn += getShift(wch);
    }
    return false;
  }

  public: bool ScanForward(
      IMatchContext* pIContext,
      Posn* inout_lPosn,
      Posn lScanStop) const {
    auto const wchFirst = getString()[0];
    auto const lLoopMax = lScanStop - m_cwch;

    auto lPosn = *inout_lPosn;
    while (lPosn <= lLoopMax) {
      auto const fFound = Case_::ForwardFindChar(
          pIContext,
          wchFirst,
          &lPosn,
          lScanStop);
      if (!fFound) {
        return false;
      }

      auto const lNextPosn = lPosn + m_cwch;

      auto const fEqual = Case_::StringEq(
          pIContext,
          getString(),
          m_cwch,
          lPosn);

      if (fEqual) {
        *inout_lPosn = lPosn;
        return true;
      }

      if (lNextPosn >= lScanStop) {
        return false;
      }

      auto const wch = Case_::Fold(pIContext, pIContext->GetChar(lNextPosn));
      lPosn += getShift(wch);
    }
    return false;
  }
};

template<class Scanner_>
class EngineWithScanner_ : public Engine {
  public: bool Backward() {
    auto const pScanner = reinterpret_cast<const Scanner_*>(m_pScanner);
    auto lPosn = m_lPosn;
    while (lPosn >= m_lScanStop) {
      if (!pScanner->ScanBackward(m_pIContext, &lPosn, m_lScanStop)) {
        break;
      }

      auto const lScan = lPosn - pScanner->GetLength();
      if (execute(lScan, lPosn)) {
        return true;
      }
      lPosn -= 1;
    }
    return false;
  }

  public: bool Forward() {
    auto const pScanner = reinterpret_cast<const Scanner_*>(m_pScanner);
    auto lPosn = m_lPosn;
    while (lPosn < m_lScanStop) {
      if (!pScanner->ScanForward(m_pIContext, &lPosn, m_lScanStop)) {
        break;
      }

      auto const lScan = lPosn + pScanner->GetLength();
      if (execute(lScan, lPosn)) {
        return true;
      }
      lPosn += 1;
    }
    return false;
  }
};

bool Engine::Execute() {
  cpush(Control_Fail);
  cpush(Control_Fail);

  if (isBackward()) {
    m_lLoopLimitPosn = m_lScanStart - 1;
    m_lScanStop = m_lScanStart + m_lMinLen;
  } else {
    m_lLoopLimitPosn = m_lScanEnd + 1;
    m_lScanStop = m_lScanEnd - m_lMinLen;
  }

  switch (m_pScanner->GetMethod()) {
    case Scanner::Method_FullBackward:
      for (Posn lPosn = m_lPosn; lPosn >= m_lScanStop; lPosn -= 1) {
        if (execute(lPosn)) return true;
      }
      break;

    case Scanner::Method_FullForward:
      for (Posn lPosn = m_lPosn; lPosn <= m_lScanStop; lPosn += 1) {
        if (execute(lPosn)) return true;
      }
      break;

    case Scanner::Method_CharCiBackward: {
      typedef CharScanner_<CiCompare> Scanner;
      typedef EngineWithScanner_<Scanner> EngineWithScanner;
      return reinterpret_cast<EngineWithScanner*>(this)->Backward();
    }

    case Scanner::Method_CharCiForward: {
      typedef CharScanner_<CiCompare> Scanner;
      typedef EngineWithScanner_<Scanner> EngineWithScanner;
      return reinterpret_cast<EngineWithScanner*>(this)->Forward();
    }

    case Scanner::Method_CharCsBackward: {
      typedef CharScanner_<CsCompare> Scanner;
      typedef EngineWithScanner_<Scanner> EngineWithScanner;
      return reinterpret_cast<EngineWithScanner*>(this)->Backward();
    }

    case Scanner::Method_CharCsForward: {
      typedef CharScanner_<CsCompare> Scanner;
      typedef EngineWithScanner_<Scanner> EngineWithScanner;
      return reinterpret_cast<EngineWithScanner*>(this)->Forward();
    }

    case Scanner::Method_StringCiBackward: {
      typedef StringScanner_<CiCompare> Scanner;
      typedef EngineWithScanner_<Scanner> EngineWithScanner;
      return reinterpret_cast<EngineWithScanner*>(this)->Backward();
    }

    case Scanner::Method_StringCiForward: {
      typedef StringScanner_<CiCompare> Scanner;
      typedef EngineWithScanner_<Scanner> EngineWithScanner;
      return reinterpret_cast<EngineWithScanner*>(this)->Forward();
    }

    case Scanner::Method_StringCsBackward: {
      typedef StringScanner_<CsCompare> Scanner;
      typedef EngineWithScanner_<Scanner> EngineWithScanner;
      return reinterpret_cast<EngineWithScanner*>(this)->Backward();
    }

    case Scanner::Method_StringCsForward: {
      typedef StringScanner_<CsCompare> Scanner;
      typedef EngineWithScanner_<Scanner> EngineWithScanner;

      return reinterpret_cast<EngineWithScanner*>(this)->Forward();
    }

    case Scanner::Method_ZeroWidth: {
      auto const pScanner = reinterpret_cast<const ZeroWidthScanner*>(
          m_pScanner);

      switch (pScanner->GetOp()) {
        case Op_AfterNewline:
          if (m_lPosn == m_lStart) {
            if (execute()) {
              return true;
            }
          }

          if (isBackward()) {
            for (Posn lPosn = m_lPosn; lPosn >= m_lScanStop; lPosn -= 1) {
              auto const fFound = m_pIContext->BackwardFindCharCs(
                  Newline,
                  &lPosn,
                  m_lScanStop);
              if (!fFound) {
                return false;
              }

              lPosn += 1;
              if (execute(lPosn)) {
                return true;
              }
            }
          } else {
            for (Posn lPosn = m_lPosn; lPosn <= m_lScanStop; lPosn += 1) {
              auto const fFound = m_pIContext->ForwardFindCharCs(
                  Newline,
                  &lPosn,
                  m_lScanStop);
              if (!fFound) {
                return false;
              }

              lPosn += 1;
              if (execute(lPosn)) {
                 return true;
              }
            }
          }
          return false;

        case Op_BeforeNewline:
          if (m_lPosn == m_lEnd) {
            if (execute()) {
              return true;
            }
          }

          if (isBackward()) {
            // Note: 2008-07-12 eval1749@gmail.com
            // Current parser doesn't emit Op_BeforeNewline for backward
            // search.
            m_lPosn -= 1;
            for (Posn lPosn = m_lPosn; lPosn >= m_lScanStop; lPosn -= 1) {
              auto const fFound = m_pIContext->BackwardFindCharCs(
                  Newline,
                  &lPosn,
                  m_lScanStop);
              if (!fFound) {
                return false;
              }

              if (execute(lPosn)) {
                return true;
              }
            }
          } else {
            m_lPosn += 1;
            for (Posn lPosn = m_lPosn; lPosn <= m_lScanStop; lPosn += 1) {
              auto const fFound = m_pIContext->ForwardFindCharCs(
                  Newline,
                  &lPosn,
                  m_lScanStop);

              if (!fFound) {
                return false;
              }

              if (execute(lPosn)) {
                return true;
              }
            }
          }
          return false;

        case Op_EndOfLine:
          if (isBackward()) {
            for (Posn lPosn = m_lPosn; lPosn >= m_lScanStop; lPosn -= 1) {
              if (isEndOfLine() && execute()) {
                return true;
              }
            }
          } else {
            for (Posn lPosn = m_lPosn; lPosn <= m_lScanStop; lPosn += 1) {
              if (isEndOfLine() && execute()) {
                return true;
              }
            }
          }
          break;

        case Op_EndOfString:
          if (isBackward() ? m_lPosn >= m_lEnd : m_lPosn <= m_lEnd) {
            m_lPosn = m_lEnd;
            return execute();
          }
          break;

        case Op_Posn:
          return execute();

        case Op_StartOfString:
          if (isBackward() ? m_lPosn >= m_lStart : m_lPosn <= m_lStart) {
            m_lPosn = m_lStart;
            return execute();
          }
          break;

        default:
          NOTREACHED();
      }
      break;
    }

    default:
      NOTREACHED();
  }

  return false;
}

/// <summary>
/// Executes regex byte code from specified start position and sets
/// captures.
/// </summary>
/// <param name="lStart">
/// Position of source text where regex byte code execute.
/// </param>
/// <param name="lMatchStart">
/// Position of source text where match started.
/// </param>
/// <returns>
/// True is executes SUCCESS instruction, false otherwise.
/// </returns>
bool Engine::execute(Posn const lStart, Posn const lMatchStart) {
  value_stack_.set_count(0);
  // Control stack has two Control_Fail pushed by Execute.
  m_nCxp = 2;
  control_stack_.set_count(m_nCxp);
  m_nPc = 0;
  m_lPosn = lStart;

  makeCapturesUnbound();

  if (!execute1()) {
    makeCapturesUnbound();
    return false;
  }

  if (isBackward()) {
    m_pIContext->SetCapture(0, m_lPosn, lMatchStart);
  } else {
    m_pIContext->SetCapture(0, lMatchStart, m_lPosn);
  }

  return true;
}

/// <summary>
/// Executes regex byte code.
/// </summary>
/// <returns>
/// True if executes SUCCESS instruction, false otherwise.
/// </returns>
bool Engine::execute1() {
  tryAgain:
  RE_DEBUG_PRINTF("\n===== Execute ====================\n");

  if (dispatch()) {
    return true;
  }

  RE_DEBUG_PRINTF("\n~~~~~ Backtrack ~~~~~~~~~~~~~~~~~~~~\n");

  for (;;) {
    #if DEBUG_EXEC
      StdOutPrintf("\tCPosnStack[%03d] ", control_stack_.count());
      printControl();
      StdOutPrintf("\n");
    #endif // DEBUG_EXEC

    switch (cpop()) {
      case Control_Capture: { // nth start end
        auto const nNth = cpop();
        auto const lStart = cpop();
        auto const lEnd = cpop();
        m_pIContext->SetCapture(nNth, lStart, lEnd);
        break;
      }

      case Control_Continue: { // nextPc posn
        auto const nNextPc = cpop();
        auto const  lPosn = cpop();
        m_nPc = nNextPc;
        m_lPosn = lPosn;
        goto tryAgain;
      }

      case Control_Fail:
        return false;

      case Control_PopInt:
        vpop();
        break;

      case Control_PopPosn:
        vpop();
        break;

      case Control_PushInt: { // int
        auto const iVal = cpop();
        vpush(iVal);
        break;
      }

      case Control_PushPosn: { // posn
        auto const lPosn = cpop();
        vpush(lPosn);
        break;
      }

      case Control_Repeat_B: { // nextPc posn maxPosn
        auto const nNextPc = cpop();
        auto const lPosn = cpop() + 1;
        auto const lMaxPosn = cpop();
        if (lPosn == lMaxPosn) {
          break;
        }

        // Loop again
        control_stack_.set_count(control_stack_.count() + 4);
        m_nPc = nNextPc;
        m_lPosn = lPosn;
        control_stack_.top(2) = m_lPosn;
        goto tryAgain;
      }

      case Control_Repeat_F: { // nextPc posn minPosn
        auto const nNextPc = cpop();
        auto const lPosn = cpop() - 1;
        auto const lMinPosn = cpop();
        if (lPosn == lMinPosn) {
          break;
        }

        // Loop again
        control_stack_.set_count(control_stack_.count() + 4);
        m_nPc = nNextPc;
        m_lPosn = lPosn;
        control_stack_.top(2) = m_lPosn;
        goto tryAgain;
      }

      case Control_ResetCapture: { // nth
        auto const nNth = cpop();
        m_pIContext->ResetCapture(nNth);
        break;
      }

      case Control_SaveCxp: // cxp vsp
        m_nCxp = cpop();
        value_stack_.set_count(cpop());
        break;

      case Control_SavePosn: // posn
        m_lPosn = cpop();
        break;

      default:
        NOTREACHED();
    }
  }
}

/// <summary>
/// Find next match.
/// </summary>
/// <param name="pIContext">Find next match on this context</param>
bool RegexObj::NextMatch(Regex::IMatchContext* pIContext) const {
  auto const fBackward = 0 != (m_rgfOption & Regex::Option_Backward);
  Posn lStart, lEnd;
  if (!pIContext->GetCapture(0, &lStart, &lEnd)) {
    return false;
  }

  Engine oContext(
    pIContext,
    GetCodeStart(),
    GetScanner(),
    m_nMinLen,
    fBackward,
    fBackward ? lEnd : lStart);
  return oContext.Execute();
}

/// <summary>
/// Find the first matchmatch.
/// </summary>
/// <param name="pIContext">Find the first match on this context</param>
bool RegexObj::StartMatch(Regex::IMatchContext* pIContext) const {
  auto const fBackward = 0 != (m_rgfOption & Regex::Option_Backward);
  Engine oContext(
    pIContext,
    GetCodeStart(),
    GetScanner(),
    m_nMinLen,
    fBackward,
    fBackward ? pIContext->GetEnd() : pIContext->GetStart());
  return oContext.Execute();
}

} // RegexPrivate

using namespace RegexPrivate;

bool StartMatch(IRegex* pIRegex, Regex::IMatchContext* pIContext) {
  return reinterpret_cast<RegexObj*>(pIRegex)->StartMatch(pIContext);
}

bool NextMatch(IRegex* pIRegex, Regex::IMatchContext* pIContext) {
  return reinterpret_cast<RegexObj*>(pIRegex)->NextMatch(pIContext);
}

} // Regex
