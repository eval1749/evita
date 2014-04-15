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

#include <memory>

#include "evita/li_util.h"
#include "evita/gfx_base.h"
#include "evita/ui/controls/scroll_bar_observer.h"
#include "evita/views/content_window.h"

class Caret;
class EditPane;
class TextRenderer;
class Selection;

namespace dom {
class TextWindow;
}

namespace text {
class Buffer;
}

namespace views {
class TextRenderer;
}

namespace ui {
class KeyboardEvent;
class MouseWheelEvent;
class ScrollBar;
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
    : public views::ContentWindow, public ui::ScrollBarObserver {
  DECLARE_CASTABLE_CLASS(TextEditWindow, views::ContentWindow);

  private: typedef common::win::Point Point;
  private: typedef text::Range Range;
  private: typedef views::ContentWindow ParentClass;
  private: typedef views::TextRenderer TextRenderer;
  private: class ScrollBar;

  private: std::unique_ptr<Caret> caret_;
  private: const gfx::Graphics* m_gfx;
  private: Posn m_lCaretPosn;
  // TODO(yosi): Manage life time of selection.
  private: Selection* selection_;
  #if SUPPORT_IME
  private: bool m_fImeTarget;
  private: Posn m_lImeStart;
  private: Posn m_lImeEnd;
  #endif // SUPPORT_IME
  private: Range* m_pViewRange;
  private: std::unique_ptr<TextRenderer> text_renderer_;
  private: ui::ScrollBar* const vertical_scroll_bar_;

  // ctor/dtor
  public: explicit TextEditWindow(const dom::TextWindow& window);

  public: virtual ~TextEditWindow();

  public: text::Buffer* buffer() const;
  private: bool is_selection_active() const;

  // [C]
  public: text::Posn ComputeMotion(
      Unit unit, Count count, const gfx::PointF& point, text::Posn position);
  private: Posn computeGoalX(float, Posn);

  // [D]
  public: void DidChangeFrame();
  private: virtual void DidChangeHierarchy() override;
  private: virtual void DidHide() override;
  private: virtual void DidKillFocus(ui::Widget* focused_window) override;
  private: virtual void DidRealize() override;
  private: virtual void DidSetFocus(ui::Widget* last_focused) override;
  private: virtual void DidShow() override;

  // [E]
  public: Posn EndOfLine(Posn);

  // [G]
  private: virtual HCURSOR GetCursorAt(const Point&) const override;
  public: HWND GetScrollBar(int which) const;
  public: Posn GetEnd();
  public: Selection* GetSelection() const { return &*selection_; }
  public: Posn GetStart();

  // [H]
  public: gfx::RectF HitTestTextPosition(Posn);

  // [L]
  public: int LargeScroll(int, int, bool = true);

  // [M]
  public: virtual void MakeSelectionVisible() override;
  public: Posn MapPointToPosn(const gfx::PointF point);

  // [O]
  private: virtual void OnDraw(gfx::Graphics* gfx) override;
  private: virtual LRESULT OnMessage(uint uMsg, WPARAM wParam, LPARAM lParam);

  // [R]
  private: virtual void Redraw() override;
  private: void Render();

  // [S]
  public: int SmallScroll(int, int);
  public: Posn StartOfLine(Posn);

  // [U]
  private: void updateScreen();
  private: void updateScrollBar();

  #if SUPPORT_IME
  private: void onImeComposition(LPARAM);
  public: void Reconvert(Posn, Posn);
  private: size_t setReconvert(RECONVERTSTRING*, Posn start, Posn end);
  private: BOOL showImeCaret(SIZE, POINT);
  #endif // SUPPORT_IME

  private: virtual void DidClickLineDown() override;
  private: virtual void DidClickLineUp() override;
  private: virtual void DidClickPageDown() override;
  private: virtual void DidClickPageUp() override;
  private: virtual void DidMoveThumb(int value) override;

  // ui::Widget
  private: virtual void DidResize() override;

  // views::Window
  private: virtual bool OnIdle(int hint) override;

  DISALLOW_COPY_AND_ASSIGN(TextEditWindow);
};

#endif //!defined(INCLUDE_listener_winapp_visual_text_pane_h)
