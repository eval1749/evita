// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_vi_EditPane_h)
#define INCLUDE_evita_vi_EditPane_h

#include "evita/views/tabs/tab_content.h"

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
class EditPane final : public views::TabContent {
  DECLARE_CASTABLE_CLASS(EditPane, TabContent);

  private: typedef views::ContentWindow ContentWindow;

  public: class Box;
  private: class SplitterController;

  private: scoped_refptr<Box> root_box_;
  private: const std::unique_ptr<SplitterController> splitter_controller_;

  public: EditPane();
  public: ~EditPane() final;

  public: bool has_more_than_one_child() const;

  public: ContentWindow* GetActiveContent() const;
  private: Frame* GetFrame();
  public: void ReplaceActiveContent(ContentWindow* window);
  public: void SetContent(ContentWindow* window);
  public: void SplitHorizontally(ContentWindow* left_window,
                                 ContentWindow* new_right_window);
  public: void SplitVertically(ContentWindow* above_window,
                               ContentWindow* new_below_window);

  // ui::Widget
  private: void DidChangeBounds() final;
  private: void DidHide() final;
  private: void DidRealize() final;
  private: void DidRealizeChildWidget(Widget* new_child) final;
  private: void DidRemoveChildWidget(Widget* old_child) final;
  private: void DidSetFocus(ui::Widget* last_focused) final;
  private: void DidShow() final;
  private: HCURSOR GetCursorAt(const gfx::Point& point) const final;
  private: void OnMouseMoved(const ui::MouseEvent&) final;
  private: void OnMousePressed(const ui::MouseEvent& event) final;
  private: void OnMouseReleased(const ui::MouseEvent& event) final;
  private: void WillDestroyWidget() final;
  private: void WillRemoveChildWidget(Widget* old_child) final;

  // views::TabContent
  private: void DidEnterSizeMove() final;
  private: void DidExitSizeMove() final;
  private: const domapi::TabData* GetTabData() const final;

  DISALLOW_COPY_AND_ASSIGN(EditPane);
};

#endif //!defined(INCLUDE_evita_vi_EditPane_h)
