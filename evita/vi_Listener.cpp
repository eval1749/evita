#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - winapp - Process Gateway
// listener/winapp/ed_Gateway.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_Listener.cpp#2 $
//
#include "./vi_Listener.h"

#include "./listener.h"

#include "evita/dom/range.h"

#include "evita/dom/document.h"
#include "./vi_Selection.h"
#include <algorithm>

//////////////////////////////////////////////////////////////////////
//
// Listener::OnReady
//
void Listener::OnReady()
{
    GetBuffer()->SetReadOnly(false);
} // Listener::OnReady


//////////////////////////////////////////////////////////////////////
//
// Listener::OnReceiveText
//
void Listener::OnReceiveText(const char16* pwch, int cwch)
{
    Buffer* pBuffer = GetBuffer();

    bool fReadOnly = pBuffer->IsReadOnly();

    if (fReadOnly)
    {
        pBuffer->SetReadOnly(false);
    }

    Posn lLast = pBuffer->GetEnd();

    {
        Edit::UndoBlock oUndo(pBuffer, L"*Listener.Output");

        m_pOutput->SetText(pwch, cwch);
        m_pOutput->Collapse(Collapse_End);
    }

    Posn lBufEnd = pBuffer->GetEnd();

    for (auto& window: pBuffer->windows()) {
        Selection* pSelection = window.GetSelection();
        if (pSelection->GetStart() == lLast &&
            pSelection->GetEnd()   == lLast )
        {
            pSelection->SetRange(lBufEnd, lBufEnd);
        }
    } // for each window

    if (fReadOnly)
    {
        pBuffer->SetReadOnly(true);
    }

    uint nFlags = ::GetQueueStatus(QS_ALLEVENTS);

    if (0 == nFlags)
    {
        // Break GetMessage in main loop.
        ::PostThreadMessage(::GetCurrentThreadId(), WM_USER, 0, 0);
    }
    else
    {
        #if DEBUG_EVENT
        {
            DEBUG_PRINTF("events 0x%x 0x%x\n",
                HIWORD(nFlags),
                LOWORD(nFlags) );
        }
        #endif // DEBUG_EVENT
    } // if
} // Listener::OnReceiveText


//////////////////////////////////////////////////////////////////////
//
// Listener::OnSentText
//
void Listener::OnSentText(Count cwch)
{
    m_oSendInfo.m_lPosn += cwch;
    sendTextAux();
} // Listener::OnSentText


//////////////////////////////////////////////////////////////////////
//
// Listener::QueryColumn
//
Count Listener::QueryColumn() const
{
    Posn lLineStart;
    GetBuffer()->ComputeMotion(
        Unit_Paragraph,
        GetBuffer()->GetEnd(),
        &lLineStart );

    return GetBuffer()->GetEnd() - lLineStart;
} // Listener::QueryColumn


//////////////////////////////////////////////////////////////////////
//
// Listener::SendText
//
void Listener::SendText()
{
    Buffer* pBuffer = GetBuffer();
    Posn lStart = m_pOutput->GetEnd();
    Posn lEnd   = pBuffer->GetEnd();
    m_pOutput->SetRange(lEnd, lEnd);
    pBuffer->SetReadOnly(true);
    m_oSendInfo.m_lEnd  = lEnd;
    m_oSendInfo.m_lPosn = lStart;
    sendTextAux();
} // Listener::SendText


//////////////////////////////////////////////////////////////////////
//
// Listener::sendTextAux
//
void Listener::sendTextAux()
{
    const Count k = lengthof(m_rgwch);
    Posn lPosn = m_oSendInfo.m_lPosn;
    Posn lEnd  = std::min(m_oSendInfo.m_lEnd,  lPosn + k);

    if (lEnd == lPosn) return;

    GetBuffer()->GetText(m_rgwch, lPosn, lEnd);

    #if _DEBUG
    {
        m_rgwch[lEnd - lPosn] = 0;
        DEBUG_PRINTF("%.50s\n", m_rgwch);
    }
    #endif // _DEBUG

    // Send text to lisp thread.
    ASSERT(lEnd >= lPosn);
    ::PostThreadMessage(
        m_dwThread,
        LISTENER_WM_SENDTEXT,
        static_cast<WPARAM>(lEnd - lPosn),
        reinterpret_cast<LPARAM>(m_rgwch) );
} // Listener::sendTextAux
