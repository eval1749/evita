// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text_window.h"

#include <algorithm>

#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/trace_event/trace_event.h"
#include "evita/dom/lock.h"
#include "evita/dom/public/text_composition_data.h"
#include "evita/dom/public/view_events.h"
#include "evita/editor/application.h"
#include "evita/editor/dom_lock.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/layout/render_selection.h"
#include "evita/layout/text_view.h"
#include "evita/metrics/time_scope.h"
#include "evita/text/buffer.h"
#include "evita/text/selection.h"
#include "evita/ui/base/ime/text_composition.h"
#include "evita/ui/base/ime/text_input_client.h"
#include "evita/ui/compositor/compositor.h"
#include "evita/ui/compositor/layer.h"
#include "evita/ui/controls/scroll_bar.h"
#include "evita/ui/focus_controller.h"
#include "evita/views/metrics_view.h"

namespace views {

using layout::TextSelectionModel;

namespace {

TextSelectionModel GetTextSelectionModel(TextWindow* window,
                                         const text::Selection& selection) {
  return TextSelectionModel(
      ui::FocusController::instance()->GetSelectionState(window),
      selection.anchor_offset(), selection.focus_offset());
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextWindow
//
TextWindow::TextWindow(WindowId window_id, text::Selection* selection)
    : CanvasContentWindow(window_id),
      metrics_view_(new MetricsView()),
      text_view_(new layout::TextView(*selection->buffer(), this)),
      selection_(selection),
      vertical_scroll_bar_(
          new ui::ScrollBar(ui::ScrollBar::Type::Vertical, this)) {
  AppendChild(vertical_scroll_bar_);
  AppendChild(metrics_view_);
  UI_DOM_AUTO_LOCK_SCOPE();
  buffer().AddObserver(this);
  selection_->AddObserver(this);
}

TextWindow::~TextWindow() {
  UI_DOM_AUTO_LOCK_SCOPE();
  buffer().RemoveObserver(this);
  selection_->RemoveObserver(this);
}

const text::Buffer& TextWindow::buffer() const {
  return text_view_->buffer();
}

text::Offset TextWindow::ComputeEndOfLine(text::Offset text_offset) {
  UI_ASSERT_DOM_LOCKED();
  return text_view_->ComputeEndOfLine(text_offset);
}

text::Offset TextWindow::ComputeScreenMotion(int n,
                                             const gfx::PointF& pt,
                                             text::Offset lPosn) {
  UI_ASSERT_DOM_LOCKED();
  // TODO(eval1749): We should not call |LargetScroll()| in |ComputeMotion|.
  if (LargeScroll(0, n))
    return HitTestPoint(pt);
  if (n > 0)
    return std::min(text_view_->text_end(), buffer().GetEnd());
  if (n < 0)
    return text_view_->text_start();
  return lPosn;
}

text::Offset TextWindow::ComputeStartOfLine(text::Offset text_offset) {
  UI_ASSERT_DOM_LOCKED();
  return text_view_->ComputeStartOfLine(text_offset);
}

text::Offset TextWindow::ComputeWindowLineMotion(int n,
                                                 const gfx::PointF& pt,
                                                 text::Offset lPosn) {
  UI_ASSERT_DOM_LOCKED();
  text_view_->FormatIfNeeded();
  if (n > 0) {
    auto const lBufEnd = buffer().GetEnd();
    if (lPosn >= lBufEnd)
      return lBufEnd;
    auto lGoal = lPosn;
    auto k = 0;
    for (k = 0; k < n; ++k) {
      lGoal = ComputeEndOfLine(lGoal);
      if (lGoal >= lBufEnd)
        break;
      ++lGoal;
    }
    return text_view_->MapPointXToOffset(std::min(lGoal, lBufEnd), pt.x);
  }
  if (n < 0) {
    n = -n;
    auto const lBufStart = text::Offset(0);
    auto lStart = lPosn;
    auto k = 0;
    for (k = 0; k < n; ++k) {
      lStart = ComputeStartOfLine(lStart);
      if (lStart <= lBufStart)
        break;
      --lStart;
    }

    return text_view_->MapPointXToOffset(std::max(lStart, lBufStart), pt.x);
  }
  return lPosn;
}

text::Offset TextWindow::ComputeWindowMotion(int n, text::Offset offset) {
  UI_ASSERT_DOM_LOCKED();
  text_view_->FormatIfNeeded();
  if (n > 0)
    return std::max(std::min(text_view_->text_end() - text::OffsetDelta(1),
                             buffer().GetEnd()),
                    text_view_->text_start());
  if (n < 0)
    return text_view_->text_start();
  return offset;
}

text::Offset TextWindow::HitTestPoint(const gfx::PointF pt) {
  text_view_->FormatIfNeeded();
  return std::min(text_view_->HitTestPoint(pt), buffer().GetEnd());
}

// Maps position specified buffer position and returns height
// of caret, If specified buffer position isn't in window, this function
// returns 0.
gfx::RectF TextWindow::HitTestTextPosition(text::Offset text_offset) {
  DCHECK(text_offset.IsValid());
  UI_ASSERT_DOM_LOCKED();
  text_view_->FormatIfNeeded();
  return text_view_->HitTestTextPosition(text_offset);
}

bool TextWindow::LargeScroll(int, int iDy) {
  UI_ASSERT_DOM_LOCKED();
  text_view_->FormatIfNeeded();
  auto scrolled = false;
  if (iDy < 0) {
    // Scroll Down -- place top line out of window.
    iDy = -iDy;

    auto const lBufStart = text::Offset(0);
    for (auto k = 0; k < iDy; ++k) {
      auto const lStart = text_view_->text_start();
      if (lStart == lBufStart)
        break;

      // Scroll down until page start goes out to page.
      do {
        if (!text_view_->ScrollDown())
          break;
        scrolled = true;
      } while (text_view_->text_end() != lStart);
    }
  } else if (iDy > 0) {
    // Scroll Up -- format page from page end.
    auto const lBufEnd = buffer().GetEnd();
    for (auto k = 0; k < iDy; ++k) {
      auto const lStart = text_view_->text_end();
      if (lStart >= lBufEnd)
        break;
      text_view_->Format(lStart);
      scrolled = true;
    }
  }
  if (scrolled)
    RequestAnimationFrame();
  return scrolled;
}

void TextWindow::SetZoom(float new_zoom) {
  text_view_->SetZoom(new_zoom);
  // TODO(eval1749) If |new_zoom| equals to old zoom value, we should not
  // request animation frame.
  RequestAnimationFrame();
}

bool TextWindow::SmallScroll(int, int y_count) {
  UI_ASSERT_DOM_LOCKED();

  auto scrolled = false;
  if (y_count < 0) {
    for (auto k = y_count; k; ++k) {
      if (!text_view_->ScrollDown())
        break;
      scrolled = true;
    }
  } else if (y_count > 0) {
    for (auto k = 0; k < y_count; ++k) {
      if (!text_view_->ScrollUp())
        break;
      scrolled = true;
    }
  }

  if (scrolled)
    RequestAnimationFrame();
  return scrolled;
}

void TextWindow::UpdateBounds() {
  DCHECK(!bounds().empty());
  auto const canvas_bounds = GetContentsBounds();
  if (canvas())
    canvas()->SetBounds(canvas_bounds);

  auto const vertical_scroll_bar_width =
      static_cast<float>(::GetSystemMetrics(SM_CXVSCROLL));

  auto const text_block_bounds = gfx::RectF(
      canvas_bounds.size() - gfx::SizeF(vertical_scroll_bar_width, 0.0f));
  text_view_->SetBounds(text_block_bounds);

  // Place vertical scroll bar at right edge of text block.
  auto const vertical_scroll_bar_bounds = gfx::RectF(
      gfx::PointF(text_block_bounds.right, text_block_bounds.top),
      gfx::SizeF(vertical_scroll_bar_width, text_block_bounds.height()));
  vertical_scroll_bar_->SetBounds(
      gfx::ToEnclosingRect(vertical_scroll_bar_bounds));

  // Place metrics view at bottom right of text block.
  auto const metrics_view_size = gfx::SizeF(metrics_view_->bounds().width(),
                                            metrics_view_->bounds().height());
  auto const metrics_view_bounds = gfx::RectF(
      text_block_bounds.bottom_right() - metrics_view_size - gfx::SizeF(3, 3),
      metrics_view_size);
  metrics_view_->SetBounds(gfx::ToEnclosingRect(metrics_view_bounds));
}

void TextWindow::UpdateScrollBar() {
  ui::ScrollBar::Data data;
  data.minimum = 0;
  data.thumb_size = text_view_->ComputeVisibleEnd() - text_view_->text_start();
  data.thumb_value = text_view_->text_start().value();
  data.maximum = buffer().GetEnd().value() + 1;
  vertical_scroll_bar_->SetData(data);
  // TODO(eval1749): Once we have scroll bar for |ui::TextWindow|, we don't
  // need to call |CancelAnimationFrameRequest()| to cancel request by
  // |ui::ScrollBar| control
  CancelAnimationFrameRequest();
}

// text::BufferMutationObserver
void TextWindow::DidChangeStyle(const text::StaticRange& range) {
  ASSERT_DOM_LOCKED();
  text_view_->DidChangeStyle(range);
  RequestAnimationFrame();
}

void TextWindow::DidDeleteAt(const text::StaticRange& range) {
  ASSERT_DOM_LOCKED();
  text_view_->DidDeleteAt(range);
  RequestAnimationFrame();
}

void TextWindow::DidInsertBefore(const text::StaticRange& range) {
  ASSERT_DOM_LOCKED();
  text_view_->DidInsertBefore(range);
  RequestAnimationFrame();
}

// text::SelectionChangeObserver
void TextWindow::DidChangeSelection() {
  TRACE_EVENT_WITH_FLOW0("views", "TextWindow::DidChangeSelection", selection_,
                         TRACE_EVENT_FLAG_FLOW_IN);
  RequestAnimationFrame();
}

// ui::AnimationFrameHandler
void TextWindow::DidBeginAnimationFrame(const base::TimeTicks& now) {
  if (!visible())
    return;
  if (!canvas()->IsReady())
    return RequestAnimationFrame();
  TRACE_EVENT0("scheduler", "TextWindow::DidBeginAnimationFrame");
  {
    UI_DOM_AUTO_TRY_LOCK_SCOPE(dom_lock_scope);
    if (!dom_lock_scope.locked()) {
      editor::Application::instance()->NotifyViewBusy();
      RequestAnimationFrame();
      return;
    }

    editor::Application::instance()->NotifyViewReady();
    metrics_view_->RecordTime();
    MetricsView::TimingScope timing_scope(metrics_view_);
    auto const selection = GetTextSelectionModel(this, *selection_);
    text_view_->Update(selection, now);
    UpdateScrollBar();
  }
  {
    gfx::Canvas::DrawingScope drawing_scope(canvas());
    text_view_->Paint(canvas());
    OnDraw(canvas());
  }
  NotifyUpdateContent();
}

// ui::LayerOwnerDelegate
void TextWindow::DidRecreateLayer(ui::Layer* old_layer) {
  CanvasContentWindow::DidRecreateLayer(old_layer);
  if (!canvas())
    return;
  old_layer->AppendLayer(metrics_view_->RecreateLayer().release());
  layer()->AppendLayer(metrics_view_->layer());
}

// ui::ScrollBarObserver
void TextWindow::DidClickLineDown() {
  UI_DOM_AUTO_LOCK_SCOPE();
  SmallScroll(0, 1);
}

void TextWindow::DidClickLineUp() {
  UI_DOM_AUTO_LOCK_SCOPE();
  SmallScroll(0, -1);
}

void TextWindow::DidClickPageDown() {
  UI_DOM_AUTO_LOCK_SCOPE();
  LargeScroll(0, 1);
}

void TextWindow::DidClickPageUp() {
  UI_DOM_AUTO_LOCK_SCOPE();
  LargeScroll(0, -1);
}

void TextWindow::DidMoveThumb(int value) {
  UI_DOM_AUTO_LOCK_SCOPE();
  if (value < 0)
    return;
  text_view_->Format(text::Offset(value));
  RequestAnimationFrame();
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
  // Note: It is OK that hidden window have focus.
  CanvasContentWindow::DidHide();
  vertical_scroll_bar_->Hide();
  text_view_->DidHide();
}

void TextWindow::DidKillFocus(ui::Widget* will_focus_widget) {
  CanvasContentWindow::DidKillFocus(will_focus_widget);
  ui::TextInputClient::Get()->CommitComposition(this);
  ui::TextInputClient::Get()->CancelComposition(this);
  ui::TextInputClient::Get()->set_delegate(nullptr);
  RequestAnimationFrame();
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
  RequestAnimationFrame();
}

void TextWindow::DidShow() {
  CanvasContentWindow::DidShow();
  vertical_scroll_bar_->Show();
}

HCURSOR TextWindow::GetCursorAt(const gfx::Point&) const {
  return ::LoadCursor(nullptr, IDC_IBEAM);
}

// views::CanvasContentWindow
void TextWindow::MakeSelectionVisible() {
  text_view_->MakeSelectionVisible();
  RequestAnimationFrame();
}

}  // namespace views
