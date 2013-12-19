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

#include "./vi_defs.h"
#include "./vi_Pane.h"

class Buffer;
class Frame;
class TextEditWindow;

// EditPane is a container of multiple TextEditWindow windows and layouts
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

  private: enum StatusBarPart {
    StatusBarPart_Message,
    StatusBarPart_Mode,
    StatusBarPart_CodePage,
    StatusBarPart_Newline,
    StatusBarPart_LineNumber,
    StatusBarPart_Column,
    StatusBarPart_Posn,
    StatusBarPart_Insert,
  };

  private: typedef TextEditWindow Window;
  private: typedef DoubleLinkedList_<Window> Windows;

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
  private: const base::OwnPtr<SplitterController> splitter_controller_;
  private: Windows m_oWindows;

  // ctro/dtor
  public: explicit EditPane(Buffer*, Posn = 0);
  public: virtual ~EditPane();

  public: Frame& frame() const;

  // [A]
  public: virtual void Activate() override;

  // [C]
  public: void CloseAllBut(Window*);

  // [D]
  private: virtual void DidChangeParentWidget() override;
  private: virtual void DidRealize() override;
  private: virtual void DidRealizeChildWidget(const Widget&) override;
  private: virtual void DidResize() override;
  private: virtual void DidSetFocus() override;

  // [G]
  private: LeafBox* GetActiveLeafBox() const;
  public: virtual HCURSOR GetCursorAt(const Point&) const override;
  public: Window* GetActiveWindow() const;
  public: Buffer* GetBuffer() const;

  public: static const char* GetClass_() { return "EditPane"; }
  public: Window* GetFirstWindow() const { return m_oWindows.GetFirst(); }
  public: Window* GetLastWindow() const { return m_oWindows.GetLast(); }

  public: virtual int GetTitle(char16*, int) override;

  // [I]
  public: bool IsRealized() const {
    return m_eState == State_Realized;
  }

  // [M]
  public: virtual Command::KeyBindEntry* MapKey(uint) override;

  // [O]
  private: virtual void OnLeftButtonDown(uint flags, const Point&) override;
  private: virtual void OnLeftButtonUp(uint flags, const Point&) override;
  private: virtual void OnMouseMove(uint flags, const Point&) override;

  // [S]
  private: void setupStatusBar();
  public: Window* SplitHorizontally();
  public: Window* SplitVertically();

  // [U]
  public: virtual void UpdateStatusBar() override;

  // [W]
  private: virtual void WillDestroyWidget() override;
  private: virtual void WillDestroyChildWidget(const Widget& child) override;

  DISALLOW_COPY_AND_ASSIGN(EditPane);
};

#endif //!defined(INCLUDE_listener_winapp_visual_EditPane_h)
