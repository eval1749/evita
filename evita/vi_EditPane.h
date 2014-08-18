// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_vi_EditPane_h)
#define INCLUDE_evita_vi_EditPane_h

#include <unordered_set>

#include "base/memory/ref_counted.h"
#include "evita/ui/animation/animation_observer.h"
#include "evita/views/tab_content.h"

class Frame;

namespace text {
class Buffer;
}

namespace ui {
class MouseEvent;
class WindowAnimator;
}

namespace views {
class ContentWindow;
}

// EditPane is a container of multiple ContentWindow windows and layouts
// them vertically with draggable splitter.
class EditPane final : public views::TabContent, public ui::AnimationObserver {
  DECLARE_CASTABLE_CLASS(EditPane, TabContent);

  private: typedef common::win::Point Point;
  private: typedef common::win::Rect Rect;

  private: typedef views::ContentWindow ContentWindow;
  private: typedef views::ContentWindow Window;

  public: class Box;
  private: class SplitterController;

  private: std::unique_ptr<ui::WindowAnimator> window_animator_;
  private: std::unordered_set<ui::Animatable*> animated_contents_;
  private: scoped_refptr<Box> root_box_;
  private: const std::unique_ptr<SplitterController> splitter_controller_;

  public: EditPane();
  public: virtual ~EditPane();

  public: ui::WindowAnimator* window_animator() const;
  public: Frame& frame() const;
  public: bool has_more_than_one_child() const;

  // [G]
  public: Window* GetActiveWindow() const;

  public: Window* GetFirstWindow() const;
  public: Window* GetLastWindow() const;

  // [R]
  public: void ReplaceActiveContent(Window* window);

  // [S]
  public: void SetContent(Window* window);
  public: void SplitHorizontally(Window* left_window,
                                 Window* new_right_window);
  public: void SplitVertically(Window* above_window,
                               Window* new_below_window);

  // ui::AnimationObserver
  private: virtual void DidAnimate(ui::Animatable* animatable) override;

  // ui::Widget
  private: virtual void DidChangeBounds() override;
  private: virtual void DidHide() override;
  private: virtual void DidRealizeChildWidget(const Widget&) override;
  private: virtual void DidRemoveChildWidget(const Widget&) override;
  private: virtual void DidSetFocus(ui::Widget* last_focused) override;
  private: virtual void DidShow() override;
  private: virtual HCURSOR GetCursorAt(const Point&) const override;
  private: virtual void OnMouseMoved(const ui::MouseEvent&) override;
  private: virtual void OnMousePressed(const ui::MouseEvent& event) override;
  private: virtual void OnMouseReleased(const ui::MouseEvent& event) override;
  private: virtual void WillDestroyWidget() override;
  private: virtual void WillRemoveChildWidget(const Widget& child) override;

  // views::TabContent
  private: virtual const domapi::TabData* GetTabData() const override;

  DISALLOW_COPY_AND_ASSIGN(EditPane);
};

#endif //!defined(INCLUDE_evita_vi_EditPane_h)
