// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_tab_strip_delegate_h)
#define INCLUDE_evita_views_tab_strip_delegate_h

#include "base/strings/string16.h"
#include "evita/gfx/rect.h"

namespace views {

class TabContent;

//////////////////////////////////////////////////////////////////////
//
// TabStripDelegate
//
class TabStripDelegate {
  public: TabStripDelegate();
  public: virtual ~TabStripDelegate();

  public: virtual void DidDropTab(TabContent* tab_content,
                                  const gfx::Point& screen_point) = 0;
  public: virtual void DidSelectTab(int new_selected_index) = 0;
  public: virtual void RequestCloseTab(int tab_index) = 0;
  public: virtual void RequestSelectTab(int new_selected_index) = 0;

  DISALLOW_COPY_AND_ASSIGN(TabStripDelegate);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_tab_strip_delegate_h)
