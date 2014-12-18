// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_tab_strip_h)
#define INCLUDE_evita_views_tab_strip_h

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

  private: class View;
  friend class View;

  private: const std::unique_ptr<View> view_;

  public: TabStrip(TabStripDelegate* delegate);
  public: ~TabStrip() final;

  public: int number_of_tabs() const;
  public: int selected_index() const;

  public: TabContent* GetTab(int tab_index);
  public: void DeleteTab(int tab_index);
  // Insert a new tab before a tab at |tab_index|.
  public: void InsertTab(TabContent* tab_content, int new_tab_index);
  public: int NonClientHitTest(const gfx::Point& screen_point) const;
  // TODO(eval1749) Once we should revise tooltip handling, we should get rid
  // of |TabStrip::OnNotify()|.
  public: LRESULT OnNotify(NMHDR* nmhdr);
  public: void SelectTab(int tab_index);
  public: void SetTab(int tab_index, const domapi::TabData& tab_data);

  // ui::AnimationFrameHanndler
  private: void DidBeginAnimationFrame(base::Time time) final;

  // ui::Widget
  private: gfx::Size GetPreferredSize() const final;
  private: void DidChangeBounds() final;
  private: void DidRealize() final;

  DISALLOW_COPY_AND_ASSIGN(TabStrip);
};

}   // views

#endif // !defined(INCLUDE_evita_views_tab_strip_h)
