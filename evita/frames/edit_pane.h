// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_FRAMES_EDIT_PANE_H_
#define EVITA_FRAMES_EDIT_PANE_H_

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

  typedef views::ContentWindow ContentWindow;

 public:
  class Box;

  EditPane();
  ~EditPane() final;

  bool has_more_than_one_child() const;

  ContentWindow* GetActiveContent() const;
  void ReplaceActiveContent(ContentWindow* window);
  void SetContent(ContentWindow* window);
  void SplitHorizontally(ContentWindow* left_window,
                         ContentWindow* new_right_window);
  void SplitVertically(ContentWindow* above_window,
                       ContentWindow* new_below_window);

 private:
  class SplitterController;

  Frame* GetFrame();

  // ui::Widget
  void DidChangeBounds() final;
  void DidHide() final;
  void DidRealize() final;
  void DidRealizeChildWidget(Widget* new_child) final;
  void DidRemoveChildWidget(Widget* old_child) final;
  void DidSetFocus(ui::Widget* last_focused) final;
  void DidShow() final;
  HCURSOR GetCursorAt(const gfx::Point& point) const final;
  void OnMouseMoved(const ui::MouseEvent&) final;
  void OnMousePressed(const ui::MouseEvent& event) final;
  void OnMouseReleased(const ui::MouseEvent& event) final;
  void WillDestroyWidget() final;
  void WillRemoveChildWidget(Widget* old_child) final;

  // views::TabContent
  void DidEnterSizeMove() final;
  void DidExitSizeMove() final;
  const domapi::TabData* GetTabData() const final;

  scoped_refptr<Box> root_box_;
  const std::unique_ptr<SplitterController> splitter_controller_;

  DISALLOW_COPY_AND_ASSIGN(EditPane);
};

#endif  // EVITA_FRAMES_EDIT_PANE_H_
