// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_tab_strip_delegate_h)
#define INCLUDE_evita_views_tab_strip_delegate_h

enum TabBandDragAndDrop {
    kHover,
    kDrop,
    kThrow,
};

namespace views {

class TabStripDelegate {
  public: TabStripDelegate();
  public: virtual ~TabStripDelegate();

  public: virtual void DidChangeTabSelection(int new_selected_index) = 0;

  DISALLOW_COPY_AND_ASSIGN(TabStripDelegate);
};

}   // views

#endif //!defined(INCLUDE_evita_views_tab_strip_delegate_h)
