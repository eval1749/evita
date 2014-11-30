// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_window_h)
#define INCLUDE_evita_views_text_window_h

#include <memory>

#include "evita/gfx/canvas_observer.h"
#include "evita/text/buffer_mutation_observer.h"
#include "evita/text/selection_change_observer.h"
#include "evita/ui/base/ime/text_input_delegate.h"
#include "evita/ui/caret_owner.h"
#include "evita/ui/controls/scroll_bar_observer.h"
#include "evita/views/content_window.h"

namespace text {
class Buffer;
class Selection;
}

namespace ui {
class ScrollBar;
}

namespace views {
class MetricsView;
class TextRenderer;
namespace rendering {
class TextSelectionModel;
}

//////////////////////////////////////////////////////////////////////
//
// TextWindow
//
class TextWindow final : public ContentWindow,
                         private gfx::CanvasObserver,
                         private text::BufferMutationObserver,
                         private text::SelectionChangeObserver,
                         private ui::CaretOwner,
                         public ui::ScrollBarObserver,
                         public ui::TextInputDelegate {
  DECLARE_CASTABLE_CLASS(TextWindow, ContentWindow);

  private: typedef rendering::TextSelectionModel TextSelectionModel;

  private: text::Posn caret_offset_;
  private: MetricsView* metrics_view_;
  // TODO(eval1749): Manage life time of selection.
  private: text::Selection* const selection_;
  private: std::unique_ptr<TextRenderer> text_renderer_;
  private: ui::ScrollBar* const vertical_scroll_bar_;

  public: TextWindow(WindowId window_id, text::Selection* selection);
  public: virtual ~TextWindow();

  private: text::Buffer* buffer() const;

  public: text::Posn ComputeMotion(
      Unit unit, Count count, const gfx::PointF& point, text::Posn position);
  public: text::Posn EndOfLine(text::Posn offset);
  public: text::Posn GetEnd();
  public: text::Posn GetStart();
  public: gfx::RectF HitTestTextPosition(text::Posn offset);
  private: bool LargeScroll(int x_count, int y_count);
  public: text::Posn MapPointToPosition(const gfx::PointF point);
  private: void Paint(const TextSelectionModel& selection, base::Time now);
  private: void Redraw(base::Time now);
  public: void SetZoom(float new_zoom);
  public: bool SmallScroll(int x_count, int y_count);
  public: text::Posn StartOfLine(text::Posn offset);
  private: void UpdateLayout();

  // gfx::CanvasObserver
  private: virtual void DidRecreateCanvas() override;

  // text::BufferMutationObserver
  private: virtual void DidChangeStyle(Posn offset, size_t length) override;
  private: virtual void DidDeleteAt(Posn offset, size_t length) override;
  private: virtual void DidInsertAt(Posn offset, size_t length) override;

  // text::SelectionChangeObserver
  private: virtual void DidChangeSelection() override;

  // ui::AnimationFrameHandler
  private: virtual void DidBeginAnimationFrame(base::Time time) override;

  // ui::CaretOwner
  private: virtual void DidFireCaretTimer() override;

  // ui::LayerOwnerDelegate
  private: virtual void DidRecreateLayer(ui::Layer* old_layer) override;

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
  private: virtual void DidChangeBounds() override;
  private: virtual void DidHide() override;
  private: virtual void DidKillFocus(ui::Widget* focused_window) override;
  private: virtual void DidRealize() override;
  private: virtual void DidSetFocus(ui::Widget* last_focused) override;
  private: virtual void DidShow() override;
  private: virtual HCURSOR GetCursorAt(const gfx::Point& point) const override;

  // views::ContentWindow
  private: virtual void MakeSelectionVisible() override;

  DISALLOW_COPY_AND_ASSIGN(TextWindow);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_text_window_h)
