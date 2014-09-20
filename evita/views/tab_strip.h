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

class TabStrip : public ui::AnimatableWindow {
  DECLARE_CASTABLE_CLASS(TabStrip, AnimatableWindow);

  private: class Impl;
  friend class Impl;

  private: const std::unique_ptr<Impl> impl_;

  public: TabStrip(TabStripDelegate* delegate);
  public: virtual ~TabStrip();

  public: int number_of_tabs() const;
  public: int selected_index() const;

  public: TabContent* GetTab(int tab_index);
  public: void DeleteTab(int tab_index);
  public: void InsertTab(int new_tab_index, TabContent* tab_content);
  public: LRESULT OnNotify(NMHDR* nmhdr);
  public: void SelectTab(int tab_index);
  public: void SetTab(int tab_index, const domapi::TabData& tab_data);

  // ui::AnimationFrameHanndler
  private: virtual void DidBeginAnimationFrame(base::Time time) override;

  // ui::Widget
  private: virtual gfx::Size GetPreferredSize() const override;
  private: virtual void DidChangeBounds() override;
  private: virtual void DidRealize() override;
  private: virtual void OnMouseExited(const ui::MouseEvent& event) override;
  private: virtual void OnMouseMoved(const ui::MouseEvent& event) override;
  private: virtual void OnMousePressed(const ui::MouseEvent& event) override;
  private: virtual void OnMouseReleased(const ui::MouseEvent& event) override;

  DISALLOW_COPY_AND_ASSIGN(TabStrip);
};

}   // views

#endif //!defined(INCLUDE_evita_views_tab_strip_h)
