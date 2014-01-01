//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - edit buffer
// listener/winapp/ed_buffer.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ed_defs.h#2 $
//
#if !defined(INCLUDE_edit_defs_h)
#define INCLUDE_edit_defs_h

enum NewlineMode
{
    NewlineMode_Detect  = 0,
    NewlineMode_Lf      = 1,
    NewlineMode_Cr      = 2,
    NewlineMode_CrLf    = 3,
}; // Newline

enum CharacterName
{
    Backslash       = 0x5C,
    CloseBrace      = 0x7D,
    CloseBracket    = 0x5D,
    CloseParen      = 0x29,
    DoubleQuote     = 0x22,
    Newline         = 0x0A,
    OpenBrace       = 0x7B,
    OpenBracket     = 0x5B,
    OpenParen       = 0x28,
    SingleQuote     = 0x27,
}; // CharacterName

namespace text
{

typedef long Count;
typedef long Posn;

class Buffer;
class Range;
class Interval;

}  // namespace text

enum CollapseWhich
{
    Collapse_End   = 0,
    Collapse_Start = 1,
}; // Collapse

enum
{
    Count_Backward =  -1 << 28,
    Count_Forward  =  +1 << 28,
    Count_Max      = Count_Forward,

    Posn_Max       = Count_Max,
};

//////////////////////////////////////////////////////////////////////
//
// Unit
//
enum Unit
{
    Unit_Buffer,
    Unit_Char,
    Unit_Line,          // valid only for selection
    Unit_Page,
    Unit_Paragraph,
    Unit_Screen,        // valid only for selection
    Unit_Sentence,
    Unit_Window,        // valid only for selection
    Unit_Word,
}; // Unit

//////////////////////////////////////////////////////////////////////
//
// SearchFlag
//
enum SearchFlag
{
    SearchFlag_Backward     = 1 << 0,
    SearchFlag_CasePreserve = 1 << 1,
    SearchFlag_IgnoreCase   = 1 << 2,
    SearchFlag_MatchWord    = 1 << 3,
    SearchFlag_Regex        = 1 << 4,
    SearchFlag_Whole        = 1 << 5,
}; // SearchFlag


//////////////////////////////////////////////////////////////////////
//
// SearchParameters
//
struct SearchParameters
{
    int     m_cwch;
    void*   m_pv;
    char16  m_wsz[100];
    uint    m_rgf;

    bool IsBackward()   const { return 0 != (m_rgf & SearchFlag_Backward); }
    bool IsCasePreserve() const 
        { return 0 != (m_rgf & SearchFlag_CasePreserve); }
    bool IsIgnoreCase() const { return 0 != (m_rgf & SearchFlag_IgnoreCase); }
    bool IsMatchWord()  const { return 0 != (m_rgf & SearchFlag_MatchWord); }
    bool IsRegex()      const { return 0 != (m_rgf & SearchFlag_Regex); }
    bool IsWhole()      const { return 0 != (m_rgf & SearchFlag_Whole); }
}; // SearchParameters


enum StringCase
{
    StringCase_None,

    StringCase_Capitalized,         // "This is capitalized."
    StringCase_CapitalizedAll,      // "This Is Capitalized All."
    StringCase_Lower,               // "this is lower."
    StringCase_Mixed,               // "ThisIsMixed."
    StringCase_Upper,               // "THIS IS UPPER."
}; // String_Case

//////////////////////////////////////////////////////////////////////
//
// Utitlity Functions
//
inline char16 CharDowncase(char16 wch)
{
    return static_cast<char16>(
        reinterpret_cast<UINT_PTR>(::CharLower((char16*) wch)));
} // CharDowncase


inline char16 CharUpcase(char16 wch)
{
    return static_cast<char16>(
        reinterpret_cast<UINT_PTR>(::CharUpper((char16*) wch)));
} // CharUpcase

inline bool IsLowerCase(char16 wch)
{
    return 0 != ::IsCharLower(wch);
} // IsUpperCase

inline bool IsUpperCase(char16 wch)
{
    return 0 != ::IsCharUpper(wch);
} // IsUpperCase

inline bool IsWhitespace(char16 wch)
{
    if (0x20 == wch) return true;
    return wch >= 0x09 && wch <= 0x0C;
} // IsWhitespace

#if 0
//////////////////////////////////////////////////////////////////////
//
// String
//
class String
{
    private: int            m_cwch;
    private: const char16*  m_pwch;

    // ctor
    public: String(const char16* pwch = NULL, int cwch = 0) :
        m_cwch(cwch),
        m_pwch(pwch) {}

    public: operator const char16*() { return m_pwch; }

    // [E]
    public: class Enum
    {
        private: const char16*  m_pwch;
        private: const char16*  m_pwchEnd;

        public: Enum(const String* p) :
            m_pwchEnd(p->m_pwch + p->m_cwch),
            m_pwch(p->m_pwch) {}

        public: Enum(const String& r) :
            m_pwchEnd(r.m_pwch + r.m_cwch),
            m_pwch(r.m_pwch) {}

        public: bool AtEnd() const { return m_pwchEnd == m_pwch; }
        public: char16 Get() const { ASSERT(!AtEnd()); return *m_pwch; }
        public: void Next() { ASSERT(!AtEnd()); m_pwch++; }
    }; // Enum

    // [H]
    bool HasUpperCase() const
    {
        foreach (String::Enum, oEnum, this)
        {
            char16 wch = oEnum.Get();
            if (IsUpperCase(wch)) return true;
        } // for each char
        return false;
    } // hasUpperCase
}; // String
#endif

//////////////////////////////////////////////////////////////////////
//
// StringResult
//
// Used for Range::GetText.
//
class StringResult
{
    private: size_t     m_cwch;
    private: char16*    m_pwch;

    public: StringResult() :
        m_cwch(0),
        m_pwch(NULL) {}

    public: ~StringResult()
    {
        delete[] m_pwch;
    } // ~String

    public: operator const char16*() { return m_pwch; }

    public: char16* Alloc(size_t n)
    {
        delete[] m_pwch;
        m_cwch = n;
        return m_pwch = new char16[n + 1];
    } // Alloc
}; // StringResult

#endif //!defined(INCLUDE_edit_defs_h)
