#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - Title Bar
// listener/winapp/vi_ctrl_titleBar.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ctrl_TitleBar.cpp#1 $
//
#define DEBUG_STATUSBAR 0
#include "evita/ctrl_TitleBar.h"
#include <algorithm>

//////////////////////////////////////////////////////////////////////
//
// TitleBar::Realize
//
int TitleBar::Realize(HWND hwnd)
{
    m_hwnd = hwnd;
    return 0;
} // TitleBar::Reailize


//////////////////////////////////////////////////////////////////////
//
// TitleBar::IsEqual
//
bool TitleBar::IsEqual(const char16* pwch, int cwch) const
{
    cwch = std::min(cwch, static_cast<int>(lengthof(m_wsz) - 1));
    if (m_cwch != cwch) return false;
    return 0 == ::memcmp(m_wsz, pwch, sizeof(char16) * m_cwch);
} // TitleBar::IsEqual


//////////////////////////////////////////////////////////////////////
//
// TitleBar::SetText
//
int TitleBar::SetText(const char16* pwch, int cwch)
{
    if (IsEqual(pwch, cwch))
    {
        return 0;
    }

    m_cwch = std::min(cwch, static_cast<int>(lengthof(m_wsz) - 1));

    ::CopyMemory(m_wsz, pwch, sizeof(char16) * m_cwch);
    m_wsz[cwch] = 0;

    if (! ::SetWindowText(m_hwnd, m_wsz))
    {
        DWORD dwError = ::GetLastError();
        return static_cast<int>(dwError);
    }

    return 0;
} // TitleBar::Reailize
