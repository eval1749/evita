// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TEXT_WINDOW_H_
#define EVITA_VIEWS_TEXT_WINDOW_H_

#include <memory>

#include "evita/dom/public/float_rect.h"
#include "evita/gfx/canvas_observer.h"
#include "evita/gfx/rect_f.h"
#include "evita/ui/base/ime/text_input_delegate.h"
#include "evita/views/canvas_content_window.h"

namespace domapi {
class TextAreaDisplayItem;
}

namespace paint {
class ViewPaintCache;
}

namespace views {

class MetricsView;

//////////////////////////////////////////////////////////////////////
//
// TextWindow
//
// TextWindow manages caret by itself instead of using |ui::Caret|. See
// |PaintViewBuilder| class about caret management.
//
class TextWindow final : public CanvasContentWindow,
                         public gfx::CanvasObserver,
                         public ui::TextInputDelegate {
  DECLARE_CASTABLE_CLASS(TextWindow, ContentWindow);

  using TextAreaDisplayItem = domapi::TextAreaDisplayItem;

 public:
  explicit TextWindow(WindowId window_id);
  ~TextWindow() final;

  void Paint(std::unique_ptr<TextAreaDisplayItem> display_item);

 private:
  void UpdateBounds();

  // gfx::CanvasObserver
  void DidRecreateCanvas() final;

  // ui::AnimationFrameHandler
  void DidBeginAnimationFrame(const base::TimeTicks& time) final;

  // ui::LayerOwnerDelegate
  void DidRecreateLayer(ui::Layer* old_layer) final;

  // ui::TextInputDelegate
  void DidCommitComposition(const ui::TextComposition& composition) final;
  void DidFinishComposition() final;
  void DidStartComposition() final;
  void DidUpdateComposition(const ui::TextComposition& composition) final;
  Widget* GetClientWindow() final;

  // ui::Widget
  void DidChangeBounds() final;
  void DidHide() final;
  void DidKillFocus(ui::Widget* focused_window) final;
  void DidRealize() final;
  void DidSetFocus(ui::Widget* last_focused) final;
  void DidShow() final;
  HCURSOR GetCursorAt(const gfx::Point& point) const final;

  MetricsView* const metrics_view_;
  gfx::RectF scroll_bar_bounds_;
  std::unique_ptr<paint::ViewPaintCache> view_paint_cache_;

  DISALLOW_COPY_AND_ASSIGN(TextWindow);
};

}  // namespace views

#endif  // EVITA_VIEWS_TEXT_WINDOW_H_
