//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - Edit Pane
// listener/winapp/vi_EditPane.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_EditPane.h#2 $
//
#if !defined(INCLUDE_listener_winapp_visual_EditPane_h)
#define INCLUDE_listener_winapp_visual_EditPane_h

#include "evita/vi_defs.h"
#include "evita/vi_Pane.h"

class Frame;

namespace ui {
class MouseEvent;
}

namespace views {
class ContentWindow;
}

namespace dom {
class Buffer;
}

using Buffer = dom::Buffer;

// EditPane is a container of multiple ContentWindow windows and layouts
// them vertically with draggable splitter.
class EditPane final : public CommandWindow_<EditPane, Pane> {
  DECLARE_CASTABLE_CLASS(EditPane, Pane);

  private: enum Limits {
    k_cxSplitter = 8,
    k_cxSplitterBig = 11,
    k_cxMinBox = 50,
    k_cySplitter = 8,
    k_cySplitterBig = 11,
    k_cyMinBox = k_cySplitter,
  };

  private: typedef views::ContentWindow Window;

  private: class Box;
  private: class LayoutBox;
  private: class LeafBox;
  private: class HorizontalLayoutBox;
  private: class VerticalLayoutBox;
  private: struct HitTestResult;
  private: class SplitterController;

  private: enum State {
    State_NotRealized,
    State_Destroyed,
    State_Realized,
  };

  private: State m_eState;
  private: ScopedRefCount_<LayoutBox> root_box_;
  private: const common::OwnPtr<SplitterController> splitter_controller_;

  // ctro/dtor
  public: explicit EditPane(Buffer*, Posn = 0);
  public: explicit EditPane(Window* window);
  public: virtual ~EditPane();

  public: Frame& frame() const;

  public: bool has_more_than_one_child() const {
    return first_child() != last_child();
  }

  // [A]
  public: virtual void Activate() override;

  // [D]
  private: virtual void DidRealize() override;
  private: virtual void DidRealizeChildWidget(const Widget&) override;
  private: virtual void DidRemoveChildWidget(const Widget&) override;
  private: virtual void DidResize() override;
  private: virtual void DidSetFocus() override;

  // [G]
  private: LeafBox* GetActiveLeafBox() const;
  public: virtual HCURSOR GetCursorAt(const Point&) const override;
  public: Window* GetActiveWindow() const;
  public: Buffer* GetBuffer() const;

  public: static const char* GetClass_() { return "EditPane"; }
  public: Window* GetFirstWindow() const;
  public: Window* GetLastWindow() const;

  public: virtual int GetTitle(char16*, int) override;

  // [I]
  public: bool IsRealized() const {
    return m_eState == State_Realized;
  }

  // [M]
  public: virtual Command::KeyBindEntry* MapKey(uint) override;

  // [O]
  private: virtual void OnDraw(gfx::Graphics* gfx) override;
  private: virtual void OnMouseMoved(const ui::MouseEvent&) override;
  private: virtual void OnMousePressed(const ui::MouseEvent& event) override;
  private: virtual void OnMouseReleased(const ui::MouseEvent& event) override;

  // [R]
  public: void ReplaceActiveWindow(Window* window);

  // [S]
  public: void SplitHorizontally(Window* left_window,
                                 Window* new_right_window);
  public: void SplitVertically(Window* above_window,
                               Window* new_below_window);

  // [U]
  public: virtual void UpdateStatusBar() override;

  // [W]
  private: virtual void WillDestroyWidget() override;
  private: virtual void WillRemoveChildWidget(const Widget& child) override;

  DISALLOW_COPY_AND_ASSIGN(EditPane);
};

#endif //!defined(INCLUDE_listener_winapp_visual_EditPane_h)
