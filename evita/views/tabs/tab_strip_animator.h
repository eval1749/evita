// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TABS_TAB_STRIP_ANIMATOR_H_
#define EVITA_VIEWS_TABS_TAB_STRIP_ANIMATOR_H_

#include <unordered_set>

#include "base/macros.h"

namespace ui {
class Layer;
}

namespace views {

class TabContent;
class TabStrip;

class TabStripAnimator final {
 public:
  class Action;

  explicit TabStripAnimator(TabStrip* tab_strip);
  ~TabStripAnimator();

  TabContent* active_tab_content() const { return active_tab_content_; }

  void AddTab(TabContent* tab_content);
  void DidDeleteTabContent(TabContent* tab_content);
  void RequestSelect(TabContent* new_tab_content);
  void SetTabContentLayer(ui::Layer* layer);

 private:
  friend class Action;

  void CancelCurrentAction();
  void DidFinishAction(Action* action);

  Action* action_;
  TabContent* active_tab_content_;
  ui::Layer* layer_;
  TabStrip* const tab_strip_;

  DISALLOW_COPY_AND_ASSIGN(TabStripAnimator);
};

}  // namespace views

#endif  // EVITA_VIEWS_TABS_TAB_STRIP_ANIMATOR_H_
