// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text_window.h"

#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/trace_event/trace_event.h"
#include "evita/dom/public/float_point.h"
#include "evita/dom/public/float_rect.h"
#include "evita/dom/public/float_size.h"
#include "evita/dom/public/text_area_display_item.h"
#include "evita/dom/public/text_composition_data.h"
#include "evita/dom/public/view_events.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/color_f.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/gfx/rect_f.h"
#include "evita/metrics/time_scope.h"
#include "evita/paint/public/caret.h"
#include "evita/paint/public/selection.h"
#include "evita/paint/public/view.h"
#include "evita/paint/view_paint_cache.h"
#include "evita/paint/view_painter.h"
#include "evita/text/buffer.h"
#include "evita/text/selection.h"
#include "evita/ui/base/ime/text_composition.h"
#include "evita/ui/base/ime/text_input_client.h"
#include "evita/ui/compositor/compositor.h"
#include "evita/ui/compositor/layer.h"
#include "evita/views/metrics_view.h"
#include "evita/visuals/display/display_item_list_processor.h"
#include "evita/visuals/display/public/display_item_list.h"

namespace views {

namespace {

gfx::PointF ToPointF(const domapi::FloatPoint& point) {
  return gfx::PointF(point.x(), point.y());
}

gfx::SizeF ToSizeF(const domapi::FloatSize& size) {
  return gfx::SizeF(size.width(), size.height());
}

gfx::RectF ToRectF(const domapi::FloatRect& rect) {
  return gfx::RectF(ToPointF(rect.origin()), ToSizeF(rect.size()));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextWindow
//
TextWindow::TextWindow(WindowId window_id)
    : CanvasContentWindow(window_id), metrics_view_(new MetricsView()) {
  AppendChild(metrics_view_);
}

TextWindow::~TextWindow() {}

void TextWindow::Paint(std::unique_ptr<TextAreaDisplayItem> display_item) {
  if (!visible() || !canvas()->IsReady())
    return;
  TRACE_EVENT_WITH_FLOW0("view", "TextWindow::Paint",
                         window_id(), TRACE_EVENT_FLAG_FLOW_IN);
  metrics_view_->RecordTime();
  MetricsView::TimingScope timing_scope(metrics_view_);
  if (!display_item->paint_view()->caret().is_none()) {
    ui::TextInputClient::Get()->set_caret_bounds(
        display_item->paint_view()->caret().bounds());
  }
  gfx::Canvas::DrawingScope drawing_scope(canvas());
  // Paint text area
  view_paint_cache_ = paint::ViewPainter(*display_item->paint_view())
                          .Paint(canvas(), std::move(view_paint_cache_));

  auto display_item_list = std::move(display_item->display_item_list());
  // Paint scroll bar
  visuals::DisplayItemListProcessor processor;
  processor.Paint(canvas(), std::move(display_item_list));

  NotifyUpdateContent();
}

void TextWindow::UpdateBounds() {
  DCHECK(!bounds().empty());
  view_paint_cache_.reset();
  auto const canvas_bounds = GetContentsBounds();
  if (canvas())
    canvas()->SetBounds(canvas_bounds);

  auto const vertical_scroll_bar_width =
      static_cast<float>(::GetSystemMetrics(SM_CXVSCROLL));

  auto const text_block_bounds = gfx::RectF(
      canvas_bounds.size() - gfx::SizeF(vertical_scroll_bar_width, 0.0f));

  scroll_bar_bounds_ =
      gfx::RectF(text_block_bounds.top_right(), canvas_bounds.bottom_right());

  // Place metrics view at bottom right of text block.
  auto const metrics_view_size = gfx::SizeF(metrics_view_->bounds().width(),
                                            metrics_view_->bounds().height());
  auto const metrics_view_bounds = gfx::RectF(
      text_block_bounds.bottom_right() - metrics_view_size - gfx::SizeF(3, 3),
      metrics_view_size);
  metrics_view_->SetBounds(gfx::ToEnclosingRect(metrics_view_bounds));
}

// gfx::CanvasObserver
void TextWindow::DidRecreateCanvas() {
  view_paint_cache_.reset();
}

// ui::AnimationFrameHandler
void TextWindow::DidBeginAnimationFrame(const base::TimeTicks& now) {
  // Nothing to do.
}

// ui::LayerOwnerDelegate
void TextWindow::DidRecreateLayer(ui::Layer* old_layer) {
  CanvasContentWindow::DidRecreateLayer(old_layer);
  if (!canvas())
    return;
  old_layer->AppendLayer(metrics_view_->RecreateLayer().release());
  layer()->AppendLayer(metrics_view_->layer());
}

// ui::TextInputDelegate
void TextWindow::DidCommitComposition(const ui::TextComposition& composition) {
  DispatchTextCompositionEvent(domapi::EventType::TextCompositionCommit,
                               composition);
}

void TextWindow::DidFinishComposition() {
  DispatchTextCompositionEvent(domapi::EventType::TextCompositionEnd,
                               ui::TextComposition());
}

void TextWindow::DidStartComposition() {
  DispatchTextCompositionEvent(domapi::EventType::TextCompositionStart,
                               ui::TextComposition());
}

void TextWindow::DidUpdateComposition(const ui::TextComposition& composition) {
  DispatchTextCompositionEvent(domapi::EventType::TextCompositionUpdate,
                               composition);
}

ui::Widget* TextWindow::GetClientWindow() {
  return this;
}

// ui::Widget
void TextWindow::DidChangeBounds() {
  CanvasContentWindow::DidChangeBounds();
  UpdateBounds();
}

void TextWindow::DidHide() {
  CanvasContentWindow::DidHide();
  view_paint_cache_.reset();
}

void TextWindow::DidKillFocus(ui::Widget* will_focus_widget) {
  CanvasContentWindow::DidKillFocus(will_focus_widget);
  ui::TextInputClient::Get()->CommitComposition(this);
  ui::TextInputClient::Get()->CancelComposition(this);
  ui::TextInputClient::Get()->set_delegate(nullptr);
}

void TextWindow::DidRealize() {
  UpdateBounds();
  CanvasContentWindow::DidRealize();
  layer()->AppendLayer(metrics_view_->layer());
}

void TextWindow::DidSetFocus(ui::Widget* last_focused) {
  // Note: It is OK to set focus to hidden window.
  CanvasContentWindow::DidSetFocus(last_focused);
  ui::TextInputClient::Get()->set_delegate(this);
}

void TextWindow::DidShow() {
  CanvasContentWindow::DidShow();
  canvas()->AddObserver(this);
}

HCURSOR TextWindow::GetCursorAt(const gfx::Point& point) const {
  if (scroll_bar_bounds_.Contains(gfx::PointF(point)))
    return ::LoadCursor(nullptr, IDC_ARROW);
  return ::LoadCursor(nullptr, IDC_IBEAM);
}

}  // namespace views
