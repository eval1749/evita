// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_tab_strip_delegate_h)
#define INCLUDE_evita_views_tab_strip_delegate_h

#include "base/strings/string16.h"

namespace views {

class TabStripDelegate {
  public: TabStripDelegate();
  public: virtual ~TabStripDelegate();

  public: virtual void DidClickTabCloseButton(int tab_index) = 0;
  public: virtual void DidChangeTabSelection(int new_selected_index) = 0;
  public: virtual void DidThrowTab(LPARAM lParam) = 0;
  public: virtual base::string16 GetTooltipTextForTab(int tab_index) = 0;
  public: virtual void RequestSelectTab(int new_selected_index) = 0;
  public: virtual void OnDropTab(LPARAM lParam) = 0;

  DISALLOW_COPY_AND_ASSIGN(TabStripDelegate);
};

}   // views

#endif //!defined(INCLUDE_evita_views_tab_strip_delegate_h)
