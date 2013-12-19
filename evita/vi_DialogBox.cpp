#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - winapp - Find Dialog
// listener/winapp/dlg_find.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_DialogBox.cpp#2 $
//
#include "./vi_DialogBox.h"

extern HINSTANCE g_hInstance;
extern HINSTANCE g_hResource;
extern HWND g_hwndActiveDialog;

DialogBox* DialogBox::sm_pCreate;

//////////////////////////////////////////////////////////////////////
//
// DialogBox::dialogProc
//
INT_PTR CALLBACK DialogBox::dialogProc(
    HWND    hwnd,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam )
{
    DialogBox* pDialog = reinterpret_cast<DialogBox*>(
        ::GetWindowLongPtr(hwnd, DWLP_USER) );

    if (NULL == pDialog)
    {
        pDialog = sm_pCreate;
        pDialog->m_hwnd = hwnd;

        ::SetWindowLongPtr(
            hwnd,
            DWLP_USER,
            reinterpret_cast<LONG_PTR>(pDialog) );
    }

    switch (uMsg)
    {
    case WM_ACTIVATE:
        DEBUG_PRINTF("WM_ACTIVATE %p wParam=%d\n", pDialog, wParam);
        if (WA_INACTIVE == wParam)
        {
            g_hwndActiveDialog = NULL;
        }
        else
        {
            g_hwndActiveDialog = pDialog->m_hwnd;
        }
        break;

    case WM_COMMAND:
        pDialog->onCommand(wParam, lParam);
        return 0;

    case WM_INITDIALOG:
        return pDialog->onInitDialog();
    } // swtich message

    return pDialog->onMessage(uMsg, wParam, lParam);
} // DialogBox::dialogProc


//////////////////////////////////////////////////////////////////////
//
// DialogBox::DoModeless
//
bool DialogBox::DoModeless(HWND hwndParent)
{
    sm_pCreate = this;

    m_hwnd = ::CreateDialogParam(
        g_hInstance,
        MAKEINTRESOURCE(GetTemplate()),
        hwndParent,
        dialogProc,
        reinterpret_cast<LPARAM>(this) );

    return NULL != m_hwnd;
} // DialogBox::DoModeless


//////////////////////////////////////////////////////////////////////
//
// DialogBox::onOk
//
void DialogBox::onOk()
{
    ::EndDialog(m_hwnd, IDOK);
} // DialogBox::onOk


//////////////////////////////////////////////////////////////////////
//
// DialogBox::onCancel
//
void DialogBox::onCancel()
{
    ::EndDialog(m_hwnd, IDCANCEL);
} // DialogBox::onCancel
