//////////////////////////////////////////////////////////////////////////////
//
// evcl - listener - visual - text pane
// listener/winapp/vi_text_pane.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/vi_TextEditWindow.h#2 $
//
#if !defined(INCLUDE_listener_winapp_visual_text_pane_h)
#define INCLUDE_listener_winapp_visual_text_pane_h

#include "base/ref_counted.h"
#include "./li_util.h"
#include "./gfx_base.h"
#include "./vi_CommandWindow.h"
#include "./vi_Page.h"
#include <memory>

class Buffer;
class Caret;
class EditPane;
class Page;
class Selection;

enum DragMode
{
    DragMode_None,
    DragMode_Selection,
}; // DragMode

//////////////////////////////////////////////////////////////////////
//
// TextEditWindow
//
// Member Variables:
// m_pViewRange
// A range contains the start position of window.
//
class TextEditWindow
    : public CommandWindow_<TextEditWindow>,
      public DoubleLinkedNode_<TextEditWindow>,
      public DoubleLinkedNode_<TextEditWindow, Buffer> {
  DECLARE_CASTABLE_CLASS(TextEditWindow, CommandWindow);

  private: typedef DoubleLinkedNode_<TextEditWindow> WindowItem;
  private: typedef CommandWindow ParentClass;

  protected: typedef Edit::Range Range;

  protected: struct ScrollBar {
    HWND m_hwnd;
    int m_nBar;

    ScrollBar()
        : m_hwnd(nullptr),
          m_nBar(SB_CTL) {
    }

    bool GetInfo(SCROLLINFO* pInfo) {
      return m_hwnd && ::GetScrollInfo(m_hwnd, m_nBar, pInfo);
    }

    HWND GetHwnd() const { return m_hwnd; }

    void Set(HWND hwnd, int nBar) {
      m_hwnd = hwnd;
      m_nBar = nBar;
    }

    void ShowWindow(int) const;

    void SetInfo(SCROLLINFO* pInfo, bool fRedraw) {
      if (!m_hwnd)
        return;
      ::SetScrollInfo(m_hwnd, m_nBar, pInfo, fRedraw);
    }
  };

  private: class Autoscroller;
  private: class CaretBlinker;

  private: std::unique_ptr<Autoscroller> autoscroller_;
  private: std::unique_ptr<Caret> caret_;
  private: std::unique_ptr<CaretBlinker> caret_blinker_;
  protected: DragMode m_eDragMode;
  protected: bool m_fHasFocus;
  private: const gfx::Graphics* m_gfx;
  protected: Posn m_lCaretPosn;
  protected: uint m_nActiveTick;
  protected: int m_nCharTick;
  protected: ScrollBar m_oHoriScrollBar;
  protected: ScrollBar m_oVertScrollBar;
  protected: Page* m_pPage;
  // TODO(yosi): Manage life time of selection.
  protected: Selection* selection_;
  protected: Range* m_pViewRange;
  #if SUPPORT_IME
  private: bool m_fImeTarget;
  private: Posn m_lImeStart;
  private: Posn m_lImeEnd;
  #endif // SUPPORT_IME
  protected: void* m_pvHost;

  // ctor/dtor
  public: TextEditWindow(void* pvHost, Buffer*, Posn = 0);
  public: ~TextEditWindow();

  // [A]
  public: void Activate();

  // [B]
  public: void Blink(Posn, int);

  // [C]
  public: Count ComputeMotion(Unit, Count, const gfx::PointF&, Posn*);
  protected: Posn computeGoalX(float, Posn);

  // [D]
  public: void DidChangeFrame();
  private: virtual void DidChangeParentWidget() override;
  private: virtual void DidHide() override;
  private: virtual void DidKillFocus() override;
  private: virtual void DidRealize() override;
  private: virtual void DidResize() override;
  private: virtual void DidSetFocus() override;
  private: virtual void DidShow() override;

  // [E]
  public: Posn EndOfLine(Posn);
  protected: Posn endOfLineAux(const gfx::Graphics&, Posn);

  // [F]
  protected: void format(const gfx::Graphics&, Posn);

  // [G]
  public: uint GetActiveTick() const { return m_nActiveTick; }
  public: Buffer* GetBuffer() const;
  public: HCURSOR GetCursorAt(const Point&) const;

  public: static const char* GetClass_() { return "TextEditWindow"; }

  public: Count GetColumn(Posn);
  public: Posn GetEnd();

  public: template<class T> T* GetHost() const {
    return reinterpret_cast<T*>(m_pvHost);
  }

  public: TextEditWindow* GetNext() const {
    return static_cast<const WindowItem*>(this)->GetNext();
  }

  public: TextEditWindow* GetPrev() const {
    return static_cast<const WindowItem*>(this)->GetPrev();
  }

  public: HWND GetScrollBarHwnd(int) const;
  public: Selection* GetSelection() const { return &*selection_; }
  public: Posn GetStart();
  public: size_t GetUndoSize() const;

  // [L]
  public: int LargeScroll(int, int, bool = true);

  // [M]
  public: virtual Command::KeyBindEntry* MapKey(uint) override;
  public: void MakeSelectionVisible();
  public: Posn MapPointToPosn(const gfx::PointF point);
  public: gfx::RectF MapPosnToPoint(Posn);

  // [O]
  private: virtual bool OnIdle(uint) override;
  private: virtual LRESULT OnMessage(uint uMsg, WPARAM wParam, LPARAM lParam);
  private: virtual void OnLeftButtonDown(uint, const Point&) override;
  private: virtual void OnLeftButtonUp(uint, const Point&) override;
  private: virtual void OnMouseMove(uint, const Point&) override;
  private: virtual void OnPaint(const gfx::Rect) override;
  private: void onVScroll(uint);

  // [R]
  public: void Redraw();
  protected: void redraw(bool);
  private: void Render();

  // [S]
  protected: void selectWord(Posn);
  public: void SetScrollBar(HWND, int);
  public: int SmallScroll(int, int);
  public: Posn StartOfLine(Posn);
  protected: Posn startOfLineAux(const gfx::Graphics&, Posn);
  private: void stopDrag();

  // [U]
  protected: void updateScreen();
  protected: void updateScrollBar();

  // [W]
  private: virtual void WillDestroyWidget() override;

  #if SUPPORT_IME
  private: void onImeComposition(LPARAM);
  public: void Reconvert(Posn, Posn);
  private: uint setReconvert(RECONVERTSTRING*, Posn, Posn);
  private: BOOL showImeCaret(SIZE, POINT);
  #endif // SUPPORT_IME

  DISALLOW_COPY_AND_ASSIGN(TextEditWindow);
};

typedef DoubleLinkedList_<TextEditWindow, Buffer> WindowList;

#endif //!defined(INCLUDE_listener_winapp_visual_text_pane_h)
