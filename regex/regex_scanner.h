//////////////////////////////////////////////////////////////////////////////
//
// Regex - Parse Tree Node
// regex_node.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evedit2/mainline/regex/regex_scanner.h#2 $
//
#ifndef REGEX_REGEX_SCANNER_H_
#define REGEX_REGEX_SCANNER_H_

#include "base/logging.h"
#include "regex/regex_bytecode.h"

namespace Regex {
namespace RegexPrivate {

/// <remark>
///   Scanner base class
/// </remark>
class Scanner {
 public:
  enum Method {
    Method_None,

    Method_CharCiBackward,
    Method_CharCiForward,

    Method_StringCiBackward,
    Method_StringCiForward,

    Method_CharCsBackward,
    Method_CharCsForward,

    Method_StringCsBackward,
    Method_StringCsForward,

    Method_FullBackward,
    Method_FullForward,

    Method_ZeroWidth,
  };

  Method GetMethod() const { return m_eMethod; }

 protected:
  explicit Scanner(Method eMethod) : m_eMethod(eMethod) {}

 private:
  Method m_eMethod;
};

/// <remark>
///  Character scanner
/// </remark>
class CharScanner : public Scanner {
 public:
  char16 getChar() const { return m_wch; }
  int GetLength() const { return 1; }

 protected:
  CharScanner(char16 wch, bool fBackward, bool fIgnoreCase)
      : m_wch(wch), Scanner(computeMethod(fBackward, fIgnoreCase)) {}

  /// <summary>
  ///   For CharScanner_ template
  /// </summary>
  CharScanner() : Scanner(Method_None) { NOTREACHED(); }

  /// <summary>
  ///  Compute scanner method code.
  /// </summary>
  /// <param name="fBackwrad">Ture if backward scanner</param>
  /// <param name="fIgnoreCase">Ture if case-insensitive scanner</param>
  static Method computeMethod(bool fBackward, bool fIgnoreCase) {
    return fBackward
               ? fIgnoreCase ? Method_CharCiBackward : Method_CharCsBackward
               : fIgnoreCase ? Method_CharCiForward : Method_CharCsForward;
  }

 private:
  char16 m_wch;
};

class FullScanner : public Scanner {
 protected:
  explicit FullScanner(bool fBackward)
      : Scanner(fBackward ? Method_FullBackward : Method_FullForward) {}
};

class StringScanner : public Scanner {
 public:
  int m_cwch;
  int m_nMaxChar;
  int m_nMinChar;
  int m_iShift;

 public:
  int GetLength() const { return m_cwch; }

  // ctor
 protected:
  StringScanner(bool fBackward, bool fIgnoreCase)
      : Scanner(computeMethod(fBackward, fIgnoreCase)) {}

  // For StringScanner_ template
  StringScanner() : Scanner(Method_None) { NOTREACHED(); }

 protected:
  const int* getDelta() const { return reinterpret_cast<const int*>(this + 1); }

  int getShift(char16 wch) const {
    if (wch < m_nMinChar)
      return m_iShift;
    if (wch > m_nMaxChar)
      return m_iShift;
    return getDelta()[wch - m_nMinChar];
  }

  const char16* getString() const {
    return reinterpret_cast<const char16*>(getDelta() +
                                           (m_nMaxChar - m_nMinChar + 1));
  }

 private:
  static Method computeMethod(bool fBackward, bool fIgnoreCase) {
    return fBackward
               ? fIgnoreCase ? Method_StringCiBackward : Method_StringCsBackward
               : fIgnoreCase ? Method_StringCiForward : Method_StringCsForward;
  }
};

class ZeroWidthScanner : public Scanner {
 public:
  explicit ZeroWidthScanner(Op eOp) : m_eOp(eOp), Scanner(Method_ZeroWidth) {}

  Op GetOp() const { return m_eOp; }

 private:
  Op m_eOp;
};

}  // namespace RegexPrivate
}  // namespace Regex

#endif  // REGEX_REGEX_SCANNER_H_
