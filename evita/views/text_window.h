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
  public: ~TextWindow() final;

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
  private: void DidRecreateCanvas() final;

  // text::BufferMutationObserver
  private: void DidChangeStyle(Posn offset, size_t length) final;
  private: void DidDeleteAt(Posn offset, size_t length) final;
  private: void DidInsertAt(Posn offset, size_t length) final;

  // text::SelectionChangeObserver
  private: void DidChangeSelection() final;

  // ui::AnimationFrameHandler
  private: void DidBeginAnimationFrame(base::Time time) final;

  // ui::CaretOwner
  private: void DidFireCaretTimer() final;

  // ui::LayerOwnerDelegate
  private: void DidRecreateLayer(ui::Layer* old_layer) final;

  // ui::ScrollBarObserver
  private: void DidClickLineDown() final;
  private: void DidClickLineUp() final;
  private: void DidClickPageDown() final;
  private: void DidClickPageUp() final;
  private: void DidMoveThumb(int value) final;

  // ui::TextInputDelegate
  private: void DidCommitComposition(
      const ui::TextComposition& composition) final;
  private: void DidFinishComposition() final;
  private: void DidStartComposition() final;
  private: void DidUpdateComposition(
      const ui::TextComposition& composition) final;
  private: Widget* GetClientWindow() final;

  // ui::Widget
  private: void DidChangeBounds() final;
  private: void DidHide() final;
  private: void DidKillFocus(ui::Widget* focused_window) final;
  private: void DidRealize() final;
  private: void DidSetFocus(ui::Widget* last_focused) final;
  private: void DidShow() final;
  private: HCURSOR GetCursorAt(const gfx::Point& point) const final;

  // views::ContentWindow
  private: void MakeSelectionVisible() final;

  DISALLOW_COPY_AND_ASSIGN(TextWindow);
};

}  // namespace views

#endif //!defined(INCLUDE_evita_views_text_window_h)
