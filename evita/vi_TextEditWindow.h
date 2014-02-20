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

#include "evita/views/content_window.h"
#include "evita/li_util.h"
#include "evita/gfx_base.h"
#include "evita/views/command_window.h"
#include "evita/vi_Page.h"
#include <memory>

class Caret;
class EditPane;
class Page;
class Selection;

namespace dom {
class Buffer;
class TextWindow;
}

namespace ui {
class KeyboardEvent;
class MouseWheelEvent;
}

//////////////////////////////////////////////////////////////////////
//
// TextEditWindow
//
// Member Variables:
// m_pViewRange
// A range contains the start position of window.
//
class TextEditWindow
    : public views::ContentWindow {
  DECLARE_CASTABLE_CLASS(TextEditWindow, views::ContentWindow);

  private: typedef views::ContentWindow ParentClass;
  private: typedef common::win::Point Point;
  private: typedef text::Range Range;

  private: struct ScrollBar {
    HWND m_hwnd;
    int m_nBar;

    ScrollBar()
        : m_hwnd(nullptr),
          m_nBar(SB_CTL) {
    }

    ~ScrollBar();

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

  private: std::unique_ptr<Caret> caret_;
  private: const gfx::Graphics* m_gfx;
  private: Posn m_lCaretPosn;
  private: ScrollBar m_oHoriScrollBar;
  private: ScrollBar m_oVertScrollBar;
  // TODO(yosi): Manage life time of selection.
  private: Selection* selection_;
  #if SUPPORT_IME
  private: bool m_fImeTarget;
  private: Posn m_lImeStart;
  private: Posn m_lImeEnd;
  #endif // SUPPORT_IME
  private: Range* m_pViewRange;
  private: std::unique_ptr<Page> m_pPage;

  // ctor/dtor
  public: explicit TextEditWindow(const dom::TextWindow& window);
  public: virtual ~TextEditWindow();

  // [C]
  public: Count ComputeMotion(Unit, Count, const gfx::PointF&, Posn*);
  private: Posn computeGoalX(float, Posn);

  // [D]
  public: void DidChangeFrame();
  private: virtual void DidChangeHierarchy() override;
  private: virtual void DidHide() override;
  private: virtual void DidKillFocus() override;
  private: virtual void DidRealize() override;
  private: virtual void DidSetFocus() override;
  private: virtual void DidShow() override;

  // [E]
  public: Posn EndOfLine(Posn);
  private: Posn endOfLineAux(const gfx::Graphics&, Posn);

  // [F]
  private: void format(const gfx::Graphics&, Posn);

  // [G]
  public: dom::Buffer* GetBuffer() const;
  private: virtual HCURSOR GetCursorAt(const Point&) const override;
  public: HWND GetScrollBar(int which) const;
  // TODO(yosi): We should not expose TextEdintWindow::GetTitle(). We export
  // this for EditPane.
  public: virtual base::string16 GetTitle() const override;
  public: Posn GetEnd();
  public: Selection* GetSelection() const { return &*selection_; }
  public: Posn GetStart();

  // [L]
  public: int LargeScroll(int, int, bool = true);

  // [M]
  public: virtual void MakeSelectionVisible() override;
  public: Posn MapPointToPosn(const gfx::PointF point);
  public: gfx::RectF MapPosnToPoint(Posn);

  // [O]
  private: virtual void OnDraw(gfx::Graphics* gfx) override;
  private: virtual bool OnIdle(uint) override;
  private: virtual LRESULT OnMessage(uint uMsg, WPARAM wParam, LPARAM lParam);
  private: virtual void OnMouseWheel(
      const ui::MouseWheelEvent& event) override;
  private: void onVScroll(uint);

  // [R]
  private: virtual void Redraw() override;
  private: void Render();

  // [S]
  public: void SetScrollBar(HWND, int);
  public: int SmallScroll(int, int);
  public: Posn StartOfLine(Posn);
  private: Posn startOfLineAux(const gfx::Graphics&, Posn);

  // [U]
  private: void updateScreen();
  private: void updateScrollBar();
  // TODO(yosi): We should not expose TextEdintWindow::UpdateStatusBar(). We
  // export this for EditPane.
  public: virtual void UpdateStatusBar() const override;

  #if SUPPORT_IME
  private: void onImeComposition(LPARAM);
  public: void Reconvert(Posn, Posn);
  private: size_t setReconvert(RECONVERTSTRING*, Posn start, Posn end);
  private: BOOL showImeCaret(SIZE, POINT);
  #endif // SUPPORT_IME

  // ContentWindow
  private: virtual int GetIconIndex() const override;

  DISALLOW_COPY_AND_ASSIGN(TextEditWindow);
};

#endif //!defined(INCLUDE_listener_winapp_visual_text_pane_h)
