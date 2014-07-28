// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_vi_EditPane_h)
#define INCLUDE_evita_vi_EditPane_h

#include "base/memory/ref_counted.h"
#include "evita/vi_Pane.h"

class Frame;

namespace text {
class Buffer;
}

namespace ui {
class MouseEvent;
}

namespace views {
class ContentWindow;
}

// EditPane is a container of multiple ContentWindow windows and layouts
// them vertically with draggable splitter.
class EditPane final : public Pane {
  DECLARE_CASTABLE_CLASS(EditPane, Pane);

  private: typedef common::win::Point Point;
  private: typedef common::win::Rect Rect;

  private: typedef views::ContentWindow Window;

  public: enum State {
    State_NotRealized,
    State_Destroyed,
    State_Realized,
  };

  public: class Box;
  private: class SplitterController;

  private: State m_eState;
  private: scoped_refptr<Box> root_box_;
  private: const std::unique_ptr<SplitterController> splitter_controller_;

  public: EditPane();
  public: virtual ~EditPane();

  public: Frame& frame() const;

  public: bool has_more_than_one_child() const {
    return first_child() != last_child();
  }


  // [G]
  public: Window* GetActiveWindow() const;

  public: Window* GetFirstWindow() const;
  public: Window* GetLastWindow() const;

  // [I]
  public: bool IsRealized() const {
    return m_eState == State_Realized;
  }

  // [R]
  public: void ReplaceActiveWindow(Window* window);

  // [S]
  public: void SetContent(Window* window);
  public: void SplitHorizontally(Window* left_window,
                                 Window* new_right_window);
  public: void SplitVertically(Window* above_window,
                               Window* new_below_window);

  // Pane
  private: virtual void Activate() override;
  private: virtual views::Window* GetWindow() const override;

  // ui::Widget
  private: virtual void DidChangeBounds() override;
  private: virtual void DidRealize() override;
  private: virtual void DidRealizeChildWidget(const Widget&) override;
  private: virtual void DidRemoveChildWidget(const Widget&) override;
  private: virtual void DidSetFocus(ui::Widget* last_focused) override;
  private: virtual HCURSOR GetCursorAt(const Point&) const override;
  private: virtual void OnMouseMoved(const ui::MouseEvent&) override;
  private: virtual void OnMousePressed(const ui::MouseEvent& event) override;
  private: virtual void OnMouseReleased(const ui::MouseEvent& event) override;
  private: virtual void WillDestroyWidget() override;
  private: virtual void WillRemoveChildWidget(const Widget& child) override;

  DISALLOW_COPY_AND_ASSIGN(EditPane);
};

#endif //!defined(INCLUDE_evita_vi_EditPane_h)
