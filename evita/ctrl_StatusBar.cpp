#include "precomp.h"
//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - Status Bar
// listener/winapp/vi_statusBar.cpp
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ctrl_StatusBar.cpp#1 $
//
#define DEBUG_STATUSBAR 0
#include "evita/ctrl_StatusBar.h"

#include <algorithm>

#include "base/logging.h"
#include "evita/vi_defs.h"

#pragma comment(lib, "comctl32.lib")

StatusBar::StatusBar()
    : m_hwnd(nullptr),
      m_cParts(0) {
}

StatusBar::~StatusBar() {
}

bool StatusBar::IsEqual(const int* prgiPart, int cParts) const {
  if (m_cParts != cParts)
    return false;

  return !::memcmp(m_rgiPart, prgiPart, sizeof(int) * cParts);
}

void StatusBar::Realize(HWND hwndParent, int idCtrl) {
  m_hwnd = ::CreateWindowEx(
      0,
      STATUSCLASSNAMEW,
      NULL,
      WS_CHILD | WS_VISIBLE,
      0, 0, 0, 0,
      hwndParent,
      reinterpret_cast<HMENU>(static_cast<INT_PTR>(idCtrl)),
      g_hInstance,
      NULL );

  ::GetWindowRect(m_hwnd, &m_rc);
}

void StatusBar::SetParts(const int* prgiPart, int cParts) {
  if (IsEqual(prgiPart, cParts))
    return;

  #if DEBUG_STATUSBAR
    DVLOG(0) << this << " cParts=" << cParts;
  #endif // DEBUG_STATUSBAR

  m_cParts = std::min(cParts, static_cast<int>(lengthof(m_rgiPart)));

  myCopyMemory(m_rgiPart, prgiPart, sizeof(int) * m_cParts);

  DCHECK(m_hwnd);

  ::SendMessage(
      m_hwnd,
      SB_SETPARTS,
      static_cast<WPARAM>(m_cParts),
      reinterpret_cast<LPARAM>(m_rgiPart) );

  // Erase borders
  for (auto i = 0; i < m_cParts; i++) {
    ::SendMessage(m_hwnd, SB_SETTEXT, static_cast<WPARAM>(i | SBT_NOBORDERS),
                  0 );
  }
}
