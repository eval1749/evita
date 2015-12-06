// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TEXT_WINDOW_H_
#define EVITA_VIEWS_TEXT_WINDOW_H_

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
class TextView;
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

 public:
  TextWindow(WindowId window_id, text::Selection* selection);
  ~TextWindow() final;

  text::Posn ComputeMotion(Unit unit,
                           Count count,
                           const gfx::PointF& point,
                           text::Posn position);
  text::Posn EndOfLine(text::Posn offset);
  text::Posn GetEnd();
  text::Posn GetStart();
  gfx::RectF HitTestTextPosition(text::Posn offset);
  text::Posn MapPointToPosition(const gfx::PointF point);
  void SetZoom(float new_zoom);
  bool SmallScroll(int x_count, int y_count);
  text::Posn StartOfLine(text::Posn offset);

 private:
  typedef rendering::TextSelectionModel TextSelectionModel;

  text::Buffer* buffer() const;

  bool LargeScroll(int x_count, int y_count);
  void Paint(const TextSelectionModel& selection, base::Time now);
  void Redraw(base::Time now);
  void UpdateLayout();

  // gfx::CanvasObserver
  void DidRecreateCanvas() final;

  // text::BufferMutationObserver
  void DidChangeStyle(Posn offset, size_t length) final;
  void DidDeleteAt(Posn offset, size_t length) final;
  void DidInsertAt(Posn offset, size_t length) final;

  // text::SelectionChangeObserver
  void DidChangeSelection() final;

  // ui::AnimationFrameHandler
  void DidBeginAnimationFrame(base::Time time) final;

  // ui::CaretOwner
  void DidFireCaretTimer() final;

  // ui::LayerOwnerDelegate
  void DidRecreateLayer(ui::Layer* old_layer) final;

  // ui::ScrollBarObserver
  void DidClickLineDown() final;
  void DidClickLineUp() final;
  void DidClickPageDown() final;
  void DidClickPageUp() final;
  void DidMoveThumb(int value) final;

  // ui::TextInputDelegate
  void DidCommitComposition(const ui::TextComposition& composition) final;
  void DidFinishComposition() final;
  void DidStartComposition() final;
  void DidUpdateComposition(const ui::TextComposition& composition) final;
  Widget* GetClientWindow() final;

  // ui::Widget
  void DidActivate() final;
  void DidChangeBounds() final;
  void DidHide() final;
  void DidKillFocus(ui::Widget* focused_window) final;
  void DidRealize() final;
  void DidSetFocus(ui::Widget* last_focused) final;
  void DidShow() final;
  HCURSOR GetCursorAt(const gfx::Point& point) const final;

  // views::ContentWindow
  void MakeSelectionVisible() final;

  text::Posn caret_offset_;
  MetricsView* metrics_view_;
  // TODO(eval1749): Manage life time of selection.
  text::Selection* const selection_;
  std::unique_ptr<TextView> text_view_;
  ui::ScrollBar* const vertical_scroll_bar_;

  DISALLOW_COPY_AND_ASSIGN(TextWindow);
};

}  // namespace views

#endif  // EVITA_VIEWS_TEXT_WINDOW_H_
