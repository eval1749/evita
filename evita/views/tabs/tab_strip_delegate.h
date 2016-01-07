// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TABS_TAB_STRIP_DELEGATE_H_
#define EVITA_VIEWS_TABS_TAB_STRIP_DELEGATE_H_

#include "base/macros.h"
#include "base/strings/string16.h"
#include "evita/gfx/rect.h"

namespace views {

class TabContent;

//////////////////////////////////////////////////////////////////////
//
// TabStripDelegate
//
class TabStripDelegate {
 public:
  virtual ~TabStripDelegate();

  virtual void DidDropTab(TabContent* tab_content,
                          const gfx::Point& screen_point) = 0;
  virtual void DidSelectTab(int new_selected_index) = 0;
  virtual void RequestCloseTab(int tab_index) = 0;
  virtual void RequestSelectTab(int new_selected_index) = 0;

 protected:
  TabStripDelegate();

 private:
  DISALLOW_COPY_AND_ASSIGN(TabStripDelegate);
};

}  // namespace views

#endif  // EVITA_VIEWS_TABS_TAB_STRIP_DELEGATE_H_
