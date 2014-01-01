#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - winapp - Input History
// listener/winapp/ap_input_history.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ap_input_history.cpp#1 $
//
#include "./ap_input_history.h"

#include "evita/dom/document.h"
#include "./vi_Selection.h"

//////////////////////////////////////////////////////////////////////
//
// InputHistory ctor
//
InputHistory::InputHistory() :
    m_nCurr(0),
    m_nEnd(0)
{
    myZeroMemory(m_rgpwsz, sizeof(m_rgpwsz));
} // InputHistory::InputHistory


//////////////////////////////////////////////////////////////////////
//
// InputHistory::Add
//
void InputHistory::Add(Edit::Buffer* pBuffer, Posn lStart, Posn lEnd)
{
    if (lStart >= lEnd)
    {
        // Ignore empty line.
        return;
    }

    {
        uint nIndex = (m_nEnd + lengthof(m_rgpwsz) - 1) % lengthof(m_rgpwsz);
        const char16* pwsz = m_rgpwsz[nIndex];
        if (NULL != pwsz)
        {
            bool fEqual = true;
            for (Posn lPosn = lStart; lPosn < lEnd; lPosn++)
            {
                if (0 == *pwsz) break;
                if (pBuffer->GetCharAt(lPosn) != *pwsz)
                {
                    fEqual = false;
                    break;
                } // if
                pwsz++;
            } // for posn
            if (fEqual)
            {
                // Specified input is as same as the last one.
                // We don't record it.
                m_nCurr = m_nEnd;
                return;
            }
        } // if
    }

    uint nIndex = m_nEnd % lengthof(m_rgpwsz);
    delete[] m_rgpwsz[nIndex];
    char16* pwsz = new char16[static_cast<size_t>(lEnd - lStart + 1)];
    m_rgpwsz[nIndex] = pwsz;
    pBuffer->GetText(pwsz, lStart, lEnd);
    pwsz[lEnd - lStart] = 0;
    m_nEnd += 1;
    m_nCurr = m_nEnd;
} // InputHistory::Add


//////////////////////////////////////////////////////////////////////
//
// InputHistory::Backward
//
void InputHistory::Backward(Selection* pSel)
{
    if (m_nCurr == 0) return;
    char16* pwsz = m_rgpwsz[(m_nCurr - 1) % lengthof(m_rgpwsz)];
    if (NULL == pwsz) return;
    m_nCurr -= 1;
    {
        Edit::DisableUndo oDisable(pSel->GetBuffer());
        pSel->SetEnd(pSel->GetBuffer()->GetEnd());
        pSel->SetText(pwsz, ::lstrlenW(pwsz));
        pSel->Collapse();
    }
} // InputHistory::Backward


//////////////////////////////////////////////////////////////////////
//
// InputHistory::Forward
//
void InputHistory::Forward(Selection* pSel)
{
    if (m_nCurr == m_nEnd) return;
    char16* pwsz = m_rgpwsz[(m_nCurr + 1) % lengthof(m_rgpwsz)];
    if (NULL == pwsz)
    {
        pwsz = L"";
    }
    else
    {
        m_nCurr += 1;
    }

    {
        Edit::DisableUndo oDisable(pSel->GetBuffer());
        pSel->SetEnd(pSel->GetBuffer()->GetEnd());
        pSel->SetText(pwsz, ::lstrlenW(pwsz));
        pSel->Collapse();
    }
} // InputHistory::Forward
