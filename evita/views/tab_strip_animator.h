// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_tab_strip_animator_h)
#define INCLUDE_evita_views_tab_strip_animator_h

#include <unordered_set>

#include "base/basictypes.h"

namespace ui {
class Layer;
}

namespace views {

class TabContent;
class TabStrip;

class TabStripAnimator final {
  public: class Action;
  friend class Action;

  private: Action* action_;
  private: TabContent* active_tab_content_;
  private: ui::Layer* layer_;
  private: TabStrip* const tab_strip_;

  public: TabStripAnimator(TabStrip* tab_strip);
  public: ~TabStripAnimator();

  public: TabContent* active_tab_content() const { return active_tab_content_; }

  public: void AddTab(TabContent* tab_content);
  private: void CancelCurrentAction();
  public: void DidDeleteTabContent(TabContent* tab_content);
  private: void DidFinishAction(Action* action);
  private: void RegisterAction(Action* action);
  public: void RequestSelect(TabContent* new_tab_content);
  public: void SetLayer(ui::Layer* layer);

  DISALLOW_COPY_AND_ASSIGN(TabStripAnimator);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_tab_strip_animator_h)
