#include "precomp.h"
// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ctrl_TitleBar.h"

#include <algorithm>

TitleBar::TitleBar()
    : m_cwch(0),
      m_hwnd(NULL) {
  m_wsz[0] = 0;
}

TitleBar::~TitleBar() {
}

int TitleBar::Realize(HWND hwnd) {
  m_hwnd = hwnd;
  return 0;
}

bool TitleBar::IsEqual(const char16* pwch, int cwch) const {
  cwch = std::min(cwch, static_cast<int>(lengthof(m_wsz) - 1));
  if (m_cwch != cwch) return false;
  return 0 == ::memcmp(m_wsz, pwch, sizeof(char16) * m_cwch);
}

int TitleBar::SetText(const char16* pwch, int cwch) {
  if (IsEqual(pwch, cwch))
    return 0;

  m_cwch = std::min(cwch, static_cast<int>(lengthof(m_wsz) - 1));

  ::CopyMemory(m_wsz, pwch, sizeof(char16) * m_cwch);
  m_wsz[cwch] = 0;

  if (!::SetWindowText(m_hwnd, m_wsz)) {
    auto const dwError = ::GetLastError();
    return static_cast<int>(dwError);
  }

  return 0;
}
