// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_listener_winapp_visual_text_pane_h)
#define INCLUDE_listener_winapp_visual_text_pane_h

#include <memory>

#include "evita/li_util.h"
#include "evita/gfx_base.h"
#include "evita/ui/base/ime/text_input_delegate.h"
#include "evita/ui/controls/scroll_bar_observer.h"
#include "evita/views/content_window.h"

class EditPane;
class TextRenderer;
class Selection;

namespace text {
class Buffer;
class Selection;
}

namespace views {
class TextRenderer;
namespace rendering {
class TextSelectionModel;
}
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
class TextEditWindow : private gfx::Canvas::Observer,
                       public ui::ScrollBarObserver,
                       public ui::TextInputDelegate,
                       public views::ContentWindow {
  DECLARE_CASTABLE_CLASS(TextEditWindow, views::ContentWindow);

  private: typedef common::win::Point Point;
  private: typedef text::Range Range;
  private: typedef views::ContentWindow ParentClass;
  private: typedef views::TextRenderer TextRenderer;
  private: typedef views::rendering::TextSelectionModel TextSelectionModel;
  private: class ScrollBar;

  private: std::unique_ptr<gfx::Canvas> canvas_;
  private: Posn m_lCaretPosn;
  // TODO(yosi): Manage life time of selection.
  private: text::Selection* const selection_;
  private: std::unique_ptr<TextRenderer> text_renderer_;
  private: ui::ScrollBar* const vertical_scroll_bar_;

  // ctor/dtor
  public: explicit TextEditWindow(views::WindowId window_id,
                                  text::Selection* selection);
  public: virtual ~TextEditWindow();

  public: text::Buffer* buffer() const;

  // [C]
  public: text::Posn ComputeMotion(
      Unit unit, Count count, const gfx::PointF& point, text::Posn position);

  // [E]
  public: Posn EndOfLine(Posn);

  // [G]
  public: Posn GetEnd();
  public: Posn GetStart();

  // [H]
  public: gfx::RectF HitTestTextPosition(Posn);

  // [L]
  public: bool LargeScroll(int x_count, int y_count);

  // [M]
  public: Posn MapPointToPosition(const gfx::PointF point);

  // [R]
  private: void Redraw();
  private: void Render(const TextSelectionModel& selection);

  // [S]
  public: bool SmallScroll(int x_count, int y_count);
  public: void SetZoom(float new_zoom);
  public: Posn StartOfLine(Posn);

  // [U]
  private: void UpdateLayout();

  // gfx::Canvas::Observer
  private: virtual void ShouldDiscardResources() override;

  // ui::ScrollBarObserver
  private: virtual void DidClickLineDown() override;
  private: virtual void DidClickLineUp() override;
  private: virtual void DidClickPageDown() override;
  private: virtual void DidClickPageUp() override;
  private: virtual void DidMoveThumb(int value) override;

  // ui::TextInputDelegate
  private: virtual void DidCommitComposition(
      const ui::TextComposition& composition) override;
  private: virtual void DidFinishComposition() override;
  private: virtual void DidStartComposition() override;
  private: virtual void DidUpdateComposition(
      const ui::TextComposition& composition) override;
  private: virtual Widget* GetClientWindow() override;

  // ui::Widget
  private: virtual void DidChangeHierarchy() override;
  private: virtual void DidHide() override;
  private: virtual void DidKillFocus(ui::Widget* focused_window) override;
  private: virtual void DidRealize() override;
  private: virtual void DidChangeBounds() override;
  private: virtual void DidSetFocus(ui::Widget* last_focused) override;
  private: virtual void DidShow() override;
  private: virtual HCURSOR GetCursorAt(const Point&) const override;

  // views::ContentWindow
  private: virtual void MakeSelectionVisible() override;

  // views::Window
  private: virtual bool OnIdle(int hint) override;

  DISALLOW_COPY_AND_ASSIGN(TextEditWindow);
};

#endif //!defined(INCLUDE_listener_winapp_visual_text_pane_h)
