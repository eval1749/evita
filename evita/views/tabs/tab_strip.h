// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TABS_TAB_STRIP_H_
#define EVITA_VIEWS_TABS_TAB_STRIP_H_

#include <memory>

#include "evita/ui/animation/animatable_window.h"

namespace domapi {
struct TabData;
}

namespace ui {
class MouseEvent;
}

namespace views {

using common::win::Size;
using common::win::Rect;

class TabContent;
class TabStripDelegate;

class TabStrip final : public ui::AnimatableWindow {
  DECLARE_CASTABLE_CLASS(TabStrip, AnimatableWindow);

 public:
  explicit TabStrip(TabStripDelegate* delegate);
  ~TabStrip() final;

  int number_of_tabs() const;
  int selected_index() const;

  TabContent* GetTab(int tab_index);
  void DeleteTab(int tab_index);
  // Insert a new tab before a tab at |tab_index|.
  void InsertTab(TabContent* tab_content, int new_tab_index);
  int NonClientHitTest(const gfx::Point& screen_point) const;
  // TODO(eval1749): Once we should revise tooltip handling, we should get rid
  // of |TabStrip::OnNotify()|.
  LRESULT OnNotify(NMHDR* nmhdr);
  void SelectTab(int tab_index);
  void SetTab(int tab_index, const domapi::TabData& tab_data);

 private:
  class View;
  friend class View;

  // ui::AnimationFrameHanndler
  void DidBeginAnimationFrame(const base::TimeTicks& time) final;

  // ui::Widget
  gfx::Size GetPreferredSize() const final;
  void DidChangeBounds() final;
  void DidRealize() final;

  const std::unique_ptr<View> view_;

  DISALLOW_COPY_AND_ASSIGN(TabStrip);
};

}  // namespace views

#endif  // EVITA_VIEWS_TABS_TAB_STRIP_H_
