// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_tab_content_h)
#define INCLUDE_evita_views_tab_content_h

#include "base/basictypes.h"
#include "base/observer_list.h"
#include "base/strings/string16.h"
#include "evita/ui/widget.h"

namespace domapi {
struct TabData;
}

namespace views {

class TabContentObserver;
class Window;

//////////////////////////////////////////////////////////////////////
//
// TabContent
//
class TabContent : public ui::Widget {
  DECLARE_CASTABLE_CLASS(TabContent, ui::Widget);

  private: int active_tick_;
  private: ObserverList<TabContentObserver> observers_;

  protected: TabContent();
  public: virtual ~TabContent();

  public: int active_tick() const { return active_tick_; }

  public: void AddObserver(TabContentObserver* observer);
  public: virtual void DidEnterSizeMove();
  public: virtual void DidExitSizeMove();
  public: virtual const domapi::TabData* GetTabData() const = 0;
  protected: void NotifyActivateTabContent();
  protected: void NotifyUpdateTabContent();
  public: void RemoveObserver(TabContentObserver* observer);

  // ui::Widget
  protected: virtual void DidHide() override;
  protected: virtual void DidRealize() override;
  protected: virtual void DidShow() override;

  DISALLOW_COPY_AND_ASSIGN(TabContent);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_tab_content_h)
