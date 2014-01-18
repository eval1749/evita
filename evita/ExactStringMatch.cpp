#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - Exact String Match
// listener/winapp/ExactStringMatch.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ExactStringMatch.cpp#1 $
//
#include "./ExactStringMatch.h"

// ctor
ExactStringMatch::ExactStringMatch(const SearchParameters* pSearch) :
    m_BadCharVec(pSearch),
    m_oSearch(*pSearch) {}

ExactStringMatch::~ExactStringMatch() {
}

//////////////////////////////////////////////////////////////////////
//
// ExactStringMatch::FirstMatch
//
bool ExactStringMatch::FirstMatch(IStringCursor* pICursor)
{
    m_pICursor = pICursor;
    return NextMatch();
} // ExactStringMatch::FirstMatch


//////////////////////////////////////////////////////////////////////
//
// ExactStringMatch::NextMatch
//
bool ExactStringMatch::NextMatch()
{
    int m = static_cast<int>(m_oSearch.search_text_.length());

    if (0 == m)
    {
        return true;
    }

    char16 wchFirst = m_oSearch.search_text_[0];

    if (m_oSearch.IsBackward())
    {
        wchFirst = m_oSearch.search_text_[m - 1];
        m = -m;
    }

    uint rgfMatch = 0;

    if (m_oSearch.IsIgnoreCase())
    {
        rgfMatch |= SearchFlag_IgnoreCase;
    }

    if (m_oSearch.IsMatchWord())
    {
        rgfMatch |= SearchFlag_MatchWord;
    }

    m_fMatched = false;
    for (;;)
    {
        if (m_oSearch.IsBackward())
        {
            if (! m_pICursor->FindBackward(wchFirst, rgfMatch))
            {
                break;
            }
        }
        else
        {
            if (! m_pICursor->FindForward(wchFirst, rgfMatch))
            {
                break;
            }
        }

        if (! m_pICursor->CanMove(m))
        {
            break;
        }

        m_fMatched = m_pICursor->Match(m_oSearch.search_text_.data(), m,
                                       rgfMatch);
        if (m_fMatched)
        {
            if (m_oSearch.IsBackward())
            {
                m_lMatchEnd   = m_pICursor->GetPosition();
                m_lMatchStart = m_lMatchEnd - m_oSearch.search_text_.length();
            }
            else
            {
                m_lMatchStart = m_pICursor->GetPosition();
                m_lMatchEnd   = m_lMatchStart + m_oSearch.search_text_.length();
            }
            return true;
        }

        char16 wch = m_pICursor->GetChar(m);
        int    iShift = getShift(wch);
        m_pICursor->Move(iShift);
    } // for

    return false;
} // ExactStringMatch::NextMatch
