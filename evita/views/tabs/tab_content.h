// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TABS_TAB_CONTENT_H_
#define EVITA_VIEWS_TABS_TAB_CONTENT_H_

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

 public:
  ~TabContent() override;

  int active_tick() const { return active_tick_; }

  void AddObserver(TabContentObserver* observer);
  virtual void DidEnterSizeMove();
  virtual void DidExitSizeMove();
  virtual const domapi::TabData* GetTabData() const = 0;
  void RemoveObserver(TabContentObserver* observer);

 protected:
  TabContent();

  void NotifyActivateTabContent();
  void NotifyUpdateTabContent();

  // ui::Widget
  void DidHide() override;
  void DidRealize() override;
  void DidShow() override;

 private:
  int active_tick_;
  base::ObserverList<TabContentObserver> observers_;

  DISALLOW_COPY_AND_ASSIGN(TabContent);
};

}  // namespace views

#endif  // EVITA_VIEWS_TABS_TAB_CONTENT_H_
