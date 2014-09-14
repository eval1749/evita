// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_vi_EditPane_h)
#define INCLUDE_evita_vi_EditPane_h

#include "evita/views/tab_content.h"

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
  public: virtual ~EditPane();

  public: bool has_more_than_one_child() const;

  public: ContentWindow* GetActiveContent() const;
  private: Frame* GetFrame() const;
  public: void ReplaceActiveContent(ContentWindow* window);
  public: void SetContent(ContentWindow* window);
  public: void SplitHorizontally(ContentWindow* left_window,
                                 ContentWindow* new_right_window);
  public: void SplitVertically(ContentWindow* above_window,
                               ContentWindow* new_below_window);

  // ui::Widget
  private: virtual void DidChangeBounds() override;
  private: virtual void DidHide() override;
  private: virtual void DidRealize() override;
  private: virtual void DidRealizeChildWidget(Widget* new_child) override;
  private: virtual void DidRemoveChildWidget(Widget* old_child) override;
  private: virtual void DidSetFocus(ui::Widget* last_focused) override;
  private: virtual void DidShow() override;
  private: virtual HCURSOR GetCursorAt(const gfx::Point& point) const override;
  private: virtual void OnMouseMoved(const ui::MouseEvent&) override;
  private: virtual void OnMousePressed(const ui::MouseEvent& event) override;
  private: virtual void OnMouseReleased(const ui::MouseEvent& event) override;
  private: virtual void WillDestroyWidget() override;
  private: virtual void WillRemoveChildWidget(Widget* old_child) override;

  // views::TabContent
  private: virtual void DidEnterSizeMove() override;
  private: virtual void DidExitSizeMove() override;
  private: virtual const domapi::TabData* GetTabData() const override;

  DISALLOW_COPY_AND_ASSIGN(EditPane);
};

#endif //!defined(INCLUDE_evita_vi_EditPane_h)
