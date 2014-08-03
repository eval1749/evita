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
#if !defined(INCLUDE_regex_scanner_h)
#define INCLUDE_regex_scanner_h


#include "base/logging.h"
#include "regex/regex_bytecode.h"

namespace Regex
{

namespace RegexPrivate
{

/// <remark>
///   Scanner base class
/// </remark>
class Scanner
{
    public: enum Method
    {
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
    }; // Method

    private: Method  m_eMethod;

    // ctor
    protected: Scanner(Method eMethod) :
        m_eMethod(eMethod) {}

    // [G]
    public: Method GetMethod() const
        { return m_eMethod; }
}; // Scanner

/// <remark>
///  Character scanner
/// </remark>
class CharScanner : public Scanner
{
    private: char16 m_wch;

    protected: CharScanner(char16 wch, bool fBackward, bool fIgnoreCase) :
        m_wch(wch),
        Scanner(computeMethod(fBackward, fIgnoreCase)) {}

    /// <summary>
    ///   For CharScanner_ template
    /// </summary>
    protected: CharScanner() :
        Scanner(Method_None) { NOTREACHED(); }

    /// <summary>
    ///  Compute scanner method code.
    /// </summary>
    /// <param name="fBackwrad">Ture if backward scanner</param>
    /// <param name="fIgnoreCase">Ture if case-insensitive scanner</param>
    protected: static Method computeMethod(bool fBackward, bool fIgnoreCase)
    {
        return fBackward ?
            fIgnoreCase ?
                Method_CharCiBackward :
                Method_CharCsBackward  :
            fIgnoreCase ?
                Method_CharCiForward :
                Method_CharCsForward;
    } // computeMethod

    // [G]
    public: char16 getChar()   const { return m_wch; }
    public: int    GetLength() const { return 1; }
}; // CharScanner

class FullScanner : public Scanner
{
    protected: FullScanner(bool fBackward) :
        Scanner(fBackward ? Method_FullBackward : Method_FullForward) {}
}; // FullScanner

class StringScanner : public Scanner
{
    public: int m_cwch;
    public: int m_nMaxChar;
    public: int m_nMinChar;
    public: int m_iShift;

    // ctor
    protected: StringScanner(
        bool fBackward,
        bool fIgnoreCase ) :
            Scanner(computeMethod(fBackward, fIgnoreCase)) {}

    // For StringScanner_ template
    protected: StringScanner() :
        Scanner(Method_None) { NOTREACHED(); }

    // [C]
    private: static Method computeMethod(bool fBackward, bool fIgnoreCase)
    {
        return fBackward ?
            fIgnoreCase ?
                Method_StringCiBackward :
                Method_StringCsBackward :
            fIgnoreCase ?
                Method_StringCiForward :
                Method_StringCsForward;
    } // computeMethod

    // [G]
    protected: const int* getDelta() const
        { return reinterpret_cast<const int*>(this + 1); }

    public: int GetLength() const
        { return m_cwch; }

    protected: int getShift(char16 wch) const
    {
        if (wch < m_nMinChar) return m_iShift;
        if (wch > m_nMaxChar) return m_iShift;
        return getDelta()[wch - m_nMinChar];
    } // getShift

    protected: const char16* getString() const
    {
        return reinterpret_cast<const char16*>(
            getDelta() + (m_nMaxChar - m_nMinChar + 1) );
    } // getString
}; // StringScanner

class ZeroWidthScanner : public Scanner
{
    private: Op m_eOp;

    public: ZeroWidthScanner(Op eOp) :
        m_eOp(eOp),
        Scanner(Method_ZeroWidth) {}

    public: Op GetOp() const
        { return m_eOp; }
}; // ZeroWidthScanner

} // RegexPrivate
} // Regex

#endif //!defined(INCLUDE_regex_scanner_h)
