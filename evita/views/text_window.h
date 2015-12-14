// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TEXT_WINDOW_H_
#define EVITA_VIEWS_TEXT_WINDOW_H_

#include <memory>

#include "evita/text/buffer_mutation_observer.h"
#include "evita/text/selection_change_observer.h"
#include "evita/ui/base/ime/text_input_delegate.h"
#include "evita/ui/controls/scroll_bar_observer.h"
#include "evita/views/canvas_content_window.h"

namespace layout {
class TextSelectionModel;
class TextView;
}

namespace text {
class Buffer;
class Selection;
}

namespace ui {
class ScrollBar;
}

namespace views {
class MetricsView;

//////////////////////////////////////////////////////////////////////
//
// TextWindow
//
// TextWindow manages caret by itself instead of using |ui::Caret|. See
// |LayoutViewBuilder| class about caret management.
//
class TextWindow final : public CanvasContentWindow,
                         public text::BufferMutationObserver,
                         public text::SelectionChangeObserver,
                         public ui::ScrollBarObserver,
                         public ui::TextInputDelegate {
  DECLARE_CASTABLE_CLASS(TextWindow, ContentWindow);

 public:
  TextWindow(WindowId window_id, text::Selection* selection);
  ~TextWindow() final;

  text::Posn ComputeScreenMotion(text::Count count,
                                 const gfx::PointF& point,
                                 text::Posn offset);
  text::Posn ComputeWindowLineMotion(text::Count count,
                                     const gfx::PointF& point,
                                     text::Posn offset);
  text::Posn ComputeWindowMotion(text::Count count, text::Posn offset);
  text::Posn EndOfLine(text::Posn offset);
  gfx::RectF HitTestTextPosition(text::Posn offset);
  text::Posn MapPointToPosition(const gfx::PointF point);
  void SetZoom(float new_zoom);
  bool SmallScroll(int x_count, int y_count);
  text::Posn StartOfLine(text::Posn offset);

 private:
  using TextSelectionModel = layout::TextSelectionModel;

  text::Buffer* buffer() const;

  bool LargeScroll(int x_count, int y_count);
  void UpdateBounds();
  void UpdateScrollBar();

  // text::BufferMutationObserver
  void DidChangeStyle(text::Posn offset, size_t length) final;
  void DidDeleteAt(text::Posn offset, size_t length) final;
  void DidInsertAt(text::Posn offset, size_t length) final;

  // text::SelectionChangeObserver
  void DidChangeSelection() final;

  // ui::AnimationFrameHandler
  void DidBeginAnimationFrame(base::Time time) final;

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

  MetricsView* metrics_view_;
  // TODO(eval1749): Manage life time of selection.
  text::Selection* const selection_;
  std::unique_ptr<layout::TextView> text_view_;
  ui::ScrollBar* const vertical_scroll_bar_;

  DISALLOW_COPY_AND_ASSIGN(TextWindow);
};

}  // namespace views

#endif  // EVITA_VIEWS_TEXT_WINDOW_H_
