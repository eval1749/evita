// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TABS_TAB_CONTENT_OBSERVER_H_
#define EVITA_VIEWS_TABS_TAB_CONTENT_OBSERVER_H_

#include "base/macros.h"

namespace views {

class TabContent;

class TabContentObserver {
 public:
  virtual ~TabContentObserver();

  virtual void DidActivateTabContent(TabContent* tab_content);
  virtual void DidUpdateContent(TabContent* tab_content);

 protected:
  TabContentObserver();

 private:
  DISALLOW_COPY_AND_ASSIGN(TabContentObserver);
};

}  // namespace views

#endif  // EVITA_VIEWS_TABS_TAB_CONTENT_OBSERVER_H_
