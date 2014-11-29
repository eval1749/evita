// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_tab_content_observer_h)
#define INCLUDE_evita_views_tab_content_observer_h

#include "base/basictypes.h"

namespace views {

class TabContent;

class TabContentObserver {
  protected: TabContentObserver();
  protected: virtual ~TabContentObserver();

  public: virtual void DidActivateTabContent(TabContent* tab_content);
  public: virtual void DidUpdateContent(TabContent* tab_content);

  DISALLOW_COPY_AND_ASSIGN(TabContentObserver);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_tab_content_observer_h)
