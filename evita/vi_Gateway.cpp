#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - winapp - Process Gateway
// listener/winapp/ed_Gateway.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_Gateway.cpp#2 $
//
#define DEBUG_EVENT 0
#include "./vi_Gateway.h"

#include "evita/dom/document.h"
#include "./vi_Selection.h"

#include "./ed_Range.h"

#include "./listener.h"
#include <algorithm>

extern HINSTANCE g_hInstance;

Gateway* Gateway::sm_pCreateWnd;
ATOM Gateway::sm_atomWndClass;

// Gateway ctor
Gateway::Gateway() :
    m_dwThread(0),
    m_hwnd(NULL),
    m_pBuffer(NULL),
    m_pOutput(NULL) {}


//////////////////////////////////////////////////////////////////////
//
// Gateway::onMessage
//
LRESULT Gateway::onMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //DEBUG_PRINTF("msg=0x%0x\n", uMsg);

    switch (uMsg)
    {
    case LISTENER_WM_QUERYCOLUMN:
    {
        Posn lLineStart;
        GetBuffer()->ComputeMotion(
            Unit_Paragraph,
            m_pOutput->GetEnd(),
            &lLineStart );

        return m_pOutput->GetEnd() - lLineStart;
    } // LISTENER_WM_QUERYCOLUMN

    case LISTENER_WM_SENDTEXT:
    {
        // Text from thread.
        const char16* pwch = reinterpret_cast<char16*>(lParam);
        auto const cwch = static_cast<int>(wParam);

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

        return 0;
    } // LISTENER_WM_SENDTEXT

    case LISTENER_WN_READY:
        GetBuffer()->SetReadOnly(false);
        return 0;

    case LISTENER_WN_RECEIVETEXT:
    {
        Count cwch = static_cast<Count>(wParam);
        m_oSendInfo.m_lPosn += cwch;
        sendTextAux();
        return 0;
    } // LISTENER_WN_RECEIVETEXT
    } // switch uMsg
    return ::DefWindowProc(m_hwnd, uMsg, wParam, lParam);
} // Gateway::onMessage


//////////////////////////////////////////////////////////////////////
//
// Gateway::SendText
//
void Gateway::SendText()
{
    Buffer* pBuffer = GetBuffer();
    Posn lStart = m_pOutput->GetEnd();
    Posn lEnd   = pBuffer->GetEnd();
    m_pOutput->SetRange(lEnd, lEnd);
    pBuffer->SetReadOnly(true);
    m_oSendInfo.m_lEnd  = lEnd;
    m_oSendInfo.m_lPosn = lStart;
    sendTextAux();
} // Gateway::SendText


//////////////////////////////////////////////////////////////////////
//
// Gateway::sendTextAux
//
void Gateway::sendTextAux()
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
} // Gateway::sendTextAux


//////////////////////////////////////////////////////////////////////
//
// Gateway::Start
//
uint Gateway::Start(Buffer* pBuffer, DWORD dwThread)
{
    ASSERT(0 == m_dwThread);
    ASSERT(NULL == m_pOutput);

    m_dwThread = dwThread;

    m_pBuffer = pBuffer;
    m_pOutput = pBuffer->CreateRange();

    if (0 == sm_atomWndClass)
    {
        WNDCLASSEXW oWC;
            oWC.cbSize          = sizeof(oWC);
            oWC.style           = 0;
            oWC.lpfnWndProc     = windowProc;
            oWC.cbClsExtra      = 0;
            oWC.cbWndExtra      = 0;
            oWC.hInstance       = g_hInstance;
            oWC.hIcon           = NULL;
            oWC.hCursor         = NULL;
            oWC.hbrBackground   = NULL;
            oWC.lpszMenuName    = NULL;
            oWC.lpszClassName   = L"Gateway";
            oWC.hIconSm         = NULL;

        sm_atomWndClass = ::RegisterClassExW(&oWC);

        if (0 == sm_atomWndClass)
        {
            return ::GetLastError();
        } // if
    } // if

    sm_pCreateWnd = this;

    m_hwnd = ::CreateWindowEx(
        0,                              // dwExStyle
        MAKEINTATOM(sm_atomWndClass),   // pClassName
        NULL,                           // pwszText
        0,                              // dwStyle
        0, 0,                           // x, y
        0, 0,                           // cx, cy
        HWND_MESSAGE,                   // hwndParent
        NULL,                           // hMenu
        g_hInstance,                    // hInstance
        0 );                            // lParam

    if (NULL == m_hwnd)
    {
        DWORD dwError = ::GetLastError();
        DEBUG_PRINTF("CreateWindow: %d\n", dwError);
        return dwError;
    } // if

    return 0;
} // Gateway::Start


// Gateway::windowProc
LRESULT CALLBACK Gateway::windowProc(
    HWND    hwnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam )
{
    Gateway* pWnd = reinterpret_cast<Gateway*>(
        static_cast<LONG_PTR>(::GetWindowLongPtrW(hwnd, GWLP_USERDATA)) );

    if (pWnd == NULL)
    {
        pWnd = sm_pCreateWnd;
        pWnd->m_hwnd = hwnd;
        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            static_cast<LONG>(reinterpret_cast<LONG_PTR>(pWnd)) );
    } // if NULL == pWnd

    return pWnd->onMessage(uMsg, wParam, lParam);
} // Gateway::windowProc
