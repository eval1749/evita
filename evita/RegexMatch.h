//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - Regex Matcher
// listener/winapp/RegexMatcher.h
//
// Copyright (C) 1996-2008 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/RegexMatch.h#2 $
//
#if !defined(INCLUDE_RegexMatcher_h)
#define INCLUDE_RegexMatcher_h

#include "regex/IRegex.h"

namespace Private
{
    class BufferMatchContext;
} // Private

class RegexMatcher
{
    private: struct ErrorInfo
    {
        int m_nError;
        int m_nPosn;

        ErrorInfo() :
            m_nError(0), m_nPosn(0) {}
    }; // ErrorInfo

    private: bool                           m_fMatched;
    private: ErrorInfo                      m_oErrorInfo;
    private: SearchParameters               m_oSearch;
    private: Regex::IRegex*                 m_pIRegex;
    private: Private::BufferMatchContext*   m_pMatchContext;

    // ctor
    public: RegexMatcher(
        const SearchParameters*,
        Edit::Buffer*,
        Posn,
        Posn );

    // [F]
    public: bool FirstMatch();

    // [G]
    public: int GetError(int* out_nChar)
    {
        *out_nChar = m_oErrorInfo.m_nPosn;
        return m_oErrorInfo.m_nError;
    } // GetError

    public: Edit::Range* GetMatched(const char16*, int);
    public: Edit::Range* GetMatched(int);

    // [N]
    public: bool NextMatch();

    // [R]
    public: void Replace(const char16*, int, bool);

    // [W]
    public: bool WrapMatch();
}; // RegexMatcher

#endif //!defined(INCLUDE_RegexMatcher_h)
