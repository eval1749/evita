// -*- Mode: C++ -*-
//
// Regex Application Program Interface
// regex_bytecode_h.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evedit2/mainline/regex/regex_bytecode.h#3 $
//
#if !defined(INCLUDE_regex_bytecode_h)
#define INCLUDE_regex_bytecode_h

#include "./IRegex.h"

namespace Regex
{

namespace RegexPrivate
{

class Scanner;

//////////////////////////////////////////////////////////////////////
//
// Op
//  Regex byte code
//
enum Op
{
    #define DefByteCode(mp_mnemonic, mp_operand) Op_ ## mp_mnemonic,
    #include "regex_bytecode.inc"
    Op_Limit,
}; // Op


class StringOperand
{
    public: int     m_cwch;

    public: class Enum
    {
        private: const char16*  m_pwch;
        private: const char16*  m_pwchEnd;

        public: Enum(const StringOperand* p) :
            m_pwch(p->Get()),
            m_pwchEnd(p->Get() + p->GetLength()) {}

        public: bool AtEnd() const { return m_pwch == m_pwchEnd; }
        public: char16 Get() const { ASSERT(!AtEnd()); return *m_pwch; }
        public: void Next() { ASSERT(!AtEnd()); m_pwch++; }
    }; // Enum

    // [G]
    public: const char16* Get() const
        { return reinterpret_cast<const char16*>(this + 1); }

    public: char16 Get(int nNth) const
        { return Get()[nNth]; }
        
    public: int GetLength() const 
        { return m_cwch; }
}; // StringOperand

//////////////////////////////////////////////////////////////////////
//
// RegexObj
//
class RegexObj
{
    public: struct Capture
    {
        int     m_iNth;
        char16* m_pwszName;
    }; // Capture

    public: void* operator new(size_t, void* pv)
        { return pv; }

    private: int    m_nMaxCapture;
    private: int    m_nMinLen;
    private: int    m_ofsCode;
    private: int    m_ofsScanner;
    private: int    m_rgfOption;

    // ctor
    public: RegexObj(
        int rgfOption,
        int nMaxCapture,
        int nMinLen,
        int ofsCode,
        int ofsScanner ):
            m_nMaxCapture(nMaxCapture),
            m_nMinLen(nMinLen),
            m_ofsCode(ofsCode),
            m_ofsScanner(ofsScanner),
            m_rgfOption(rgfOption) {}

    private: ~RegexObj() {}

    // [G]
    public: const int* GetCodeStart() const
    { 
        return reinterpret_cast<int*>(
            reinterpret_cast<Int>(this) + m_ofsCode );
    } // GetCodeStart

    public: int GetMaxCapture() const
        { return m_nMaxCapture; }

    public: int GetMinLen() const
        { return m_nMinLen; }

    public: const Scanner* GetScanner() const
    { 
        return reinterpret_cast<Scanner*>(
            reinterpret_cast<Int>(this) + m_ofsScanner );
    } // GetScanner

    // [I]
    public: bool IsBackward() const
        { return 0 != (m_rgfOption & Regex::Option_Backward); }

    // [N]
    public: bool NextMatch(Regex::IMatchContext*) const;

    // [S]
    public: bool StartMatch(Regex::IMatchContext*) const;

    #if _DEBUG
        public: void Describe() const;
    #endif
}; // RegexObj

} // RegexPrivate
} // Regex

#endif //!defined(INCLUDE_regex_bytecode_h)
