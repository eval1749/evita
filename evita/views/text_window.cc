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
#include "evita/dom/public/view_event.h"
#include "evita/editor/application.h"
#include "evita/editor/dom_lock.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/rect_conversions.h"
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
#include "evita/views/text/render_selection.h"
#include "evita/views/text/text_view.h"

namespace views {

using rendering::TextSelectionModel;

namespace {

text::Posn GetCaretOffset(const text::Buffer* buffer,
                          const TextSelectionModel& selection,
                          text::Posn caret_offset) {
  if (!selection.disabled())
    return selection.focus_offset();
  auto const max_offset = buffer->GetEnd();
  if (selection.start() == max_offset && selection.end() == max_offset)
    return max_offset;
  return caret_offset == -1 ? selection.focus_offset() : caret_offset;
}

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
    : ContentWindow(window_id),
      caret_offset_(-1),
      metrics_view_(new MetricsView()),
      text_view_(new TextView(selection->buffer(), this)),
      selection_(selection),
      vertical_scroll_bar_(
          new ui::ScrollBar(ui::ScrollBar::Type::Vertical, this)) {
  AppendChild(vertical_scroll_bar_);
  AppendChild(metrics_view_);
  UI_DOM_AUTO_LOCK_SCOPE();
  buffer()->AddObserver(this);
  selection_->AddObserver(this);
}

TextWindow::~TextWindow() {
  UI_DOM_AUTO_LOCK_SCOPE();
  buffer()->RemoveObserver(this);
  selection_->RemoveObserver(this);
}

text::Buffer* TextWindow::buffer() const {
  return text_view_->buffer();
}

text::Posn TextWindow::ComputeMotion(Unit eUnit,
                                     Count n,
                                     const gfx::PointF& pt,
                                     text::Posn lPosn) {
  UI_ASSERT_DOM_LOCKED();
  switch (eUnit) {
    case Unit_WindowLine:
      if (n > 0) {
        auto const lBufEnd = buffer()->GetEnd();
        auto lGoal = lPosn;
        auto k = 0;
        for (k = 0; k < n; ++k) {
          lGoal = EndOfLine(lGoal);
          if (lGoal >= lBufEnd)
            break;
          ++lGoal;
        }
        return text_view_->MapPointXToOffset(std::min(lGoal, lBufEnd), pt.x);
      }
      if (n < 0) {
        n = -n;
        auto const lBufStart = buffer()->GetStart();
        auto lStart = lPosn;
        auto k = 0;
        for (k = 0; k < n; ++k) {
          lStart = StartOfLine(lStart);
          if (lStart <= lBufStart)
            break;
          --lStart;
        }

        return text_view_->MapPointXToOffset(std::max(lStart, lBufStart), pt.x);
      }
      return lPosn;

    case Unit_Screen: {
      // TODO(eval1749): We should not call |LargetScroll()| in |ComputeMotion|.
      if (LargeScroll(0, n))
        return MapPointToPosition(pt);
      if (n > 0)
        return std::min(GetEnd(), buffer()->GetEnd());
      if (n < 0)
        return GetStart();
      return lPosn;
    }

    case Unit_Window:
      if (n > 0) {
        return std::max(std::min(GetEnd() - 1, buffer()->GetEnd()), GetStart());
      }
      if (n < 0)
        return GetStart();
      return lPosn;
  }

  LOG(ERROR) << "Unsupported unit " << eUnit;
  return lPosn;
}

Posn TextWindow::EndOfLine(text::Posn text_offset) {
  UI_ASSERT_DOM_LOCKED();
  return text_view_->EndOfLine(text_offset);
}

// For Selection.MoveDown Screen
Posn TextWindow::GetEnd() {
  UI_ASSERT_DOM_LOCKED();
  return text_view_->GetVisibleEnd();
}

// For Selection.MoveUp Screen
Posn TextWindow::GetStart() {
  UI_ASSERT_DOM_LOCKED();
  return text_view_->GetStart();
}
// Maps position specified buffer position and returns height
// of caret, If specified buffer position isn't in window, this function
// returns 0.
gfx::RectF TextWindow::HitTestTextPosition(text::Posn text_offset) {
  DCHECK_GE(text_offset, 0);
  UI_ASSERT_DOM_LOCKED();
  return text_view_->HitTestTextPosition(text_offset);
}

bool TextWindow::LargeScroll(int, int iDy) {
  UI_ASSERT_DOM_LOCKED();

  auto scrolled = false;
  if (iDy < 0) {
    // Scroll Down -- place top line out of window.
    iDy = -iDy;

    auto const lBufStart = buffer()->GetStart();
    for (auto k = 0; k < iDy; ++k) {
      auto const lStart = text_view_->GetStart();
      if (lStart == lBufStart)
        break;

      // Scroll down until page start goes out to page.
      do {
        if (!text_view_->ScrollDown())
          break;
        scrolled = true;
      } while (text_view_->GetEnd() != lStart);
    }
  } else if (iDy > 0) {
    // Scroll Up -- format page from page end.
    auto const lBufEnd = buffer()->GetEnd();
    for (auto k = 0; k < iDy; ++k) {
      auto const lStart = text_view_->GetEnd();
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

Posn TextWindow::MapPointToPosition(const gfx::PointF pt) {
  return std::min(text_view_->MapPointToPosition(pt), buffer()->GetEnd());
}

void TextWindow::Paint(const TextSelectionModel& selection, base::Time now) {
  DCHECK(visible());
  TRACE_EVENT0("view", "TextWindow::Paint");
  gfx::Canvas::DrawingScope drawing_scope(canvas());
  text_view_->Paint(canvas(), selection, now);
}

void TextWindow::Redraw(base::Time now) {
  DCHECK(visible());
  TRACE_EVENT0("view", "TextWindow::Redraw");
  MetricsView::TimingScope timing_scope(metrics_view_);
  UI_ASSERT_DOM_LOCKED();

  auto const selection = GetTextSelectionModel(this, *selection_);
  auto const new_caret_offset =
      GetCaretOffset(buffer(), selection, caret_offset_);
  DCHECK_GE(new_caret_offset, 0);

  if (text_view_->FormatIfNeeded()) {
    if (caret_offset_ != new_caret_offset) {
      text_view_->ScrollToPosition(new_caret_offset);
      caret_offset_ = new_caret_offset;
    }
    Paint(selection, now);
    return;
  }

  if (caret_offset_ != new_caret_offset) {
    caret_offset_ = new_caret_offset;
    if (text_view_->IsPositionFullyVisible(new_caret_offset)) {
      Paint(selection, now);
      return;
    }
    text_view_->ScrollToPosition(new_caret_offset);
    Paint(selection, now);
    return;
  }

  Paint(selection, now);
}

void TextWindow::SetZoom(float new_zoom) {
  text_view_->SetZoom(new_zoom);
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

Posn TextWindow::StartOfLine(text::Posn text_offset) {
  UI_ASSERT_DOM_LOCKED();
  return text_view_->StartOfLine(text_offset);
}

void TextWindow::UpdateLayout() {
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
  data.thumb_size = text_view_->GetVisibleEnd() - text_view_->GetStart();
  data.thumb_value = text_view_->GetStart();
  data.maximum = buffer()->GetEnd() + 1;
  vertical_scroll_bar_->SetData(data);
}

// gfx::CanvasObserver
void TextWindow::DidRecreateCanvas() {
  text_view_->DidRecreateCanvas();
}

// text::BufferMutationObserver
void TextWindow::DidChangeStyle(text::Posn offset, size_t length) {
  ASSERT_DOM_LOCKED();
  text_view_->DidChangeStyle(offset, length);
  RequestAnimationFrame();
}

void TextWindow::DidDeleteAt(text::Posn offset, size_t length) {
  ASSERT_DOM_LOCKED();
  text_view_->DidDeleteAt(offset, length);
  RequestAnimationFrame();
}

void TextWindow::DidInsertAt(text::Posn offset, size_t length) {
  ASSERT_DOM_LOCKED();
  text_view_->DidInsertAt(offset, length);
  RequestAnimationFrame();
}

// text::SelectionChangeObserver
void TextWindow::DidChangeSelection() {
  RequestAnimationFrame();
}

// ui::AnimationFrameHandler
void TextWindow::DidBeginAnimationFrame(base::Time now) {
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

    // TODO(eval1749): We should narrow drawing scope just enclosing |OnDraw()|.
    gfx::Canvas::DrawingScope drawing_scope(canvas());
    Redraw(now);
    TRACE_EVENT0("view", "TextWindow::DidBeginAnimationFrame/1");
    UpdateScrollBar();
    OnDraw(canvas());
  }
  NotifyUpdateContent();
}

// ui::CaretOwner
void TextWindow::DidFireCaretTimer() {
  RequestAnimationFrame();
}

// ui::LayerOwnerDelegate
void TextWindow::DidRecreateLayer(ui::Layer* old_layer) {
  ContentWindow::DidRecreateLayer(old_layer);
  if (!canvas())
    return;
  text_view_->DidRecreateCanvas();
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
  text_view_->Format(value);
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
void TextWindow::DidActivate() {
  RequestAnimationFrame();
}

void TextWindow::DidChangeBounds() {
  ContentWindow::DidChangeBounds();
  UpdateLayout();
}

void TextWindow::DidHide() {
  // Note: It is OK that hidden window have focus.
  ContentWindow::DidHide();
  vertical_scroll_bar_->Hide();
  text_view_->DidHide();
}

void TextWindow::DidKillFocus(ui::Widget* will_focus_widget) {
  ContentWindow::DidKillFocus(will_focus_widget);
  ui::TextInputClient::Get()->CommitComposition(this);
  ui::TextInputClient::Get()->CancelComposition(this);
  ui::TextInputClient::Get()->set_delegate(nullptr);
  RequestAnimationFrame();
}

void TextWindow::DidRealize() {
  UpdateLayout();
  ContentWindow::DidRealize();
  layer()->AppendLayer(metrics_view_->layer());
}

void TextWindow::DidSetFocus(ui::Widget* last_focused) {
  // Note: It is OK to set focus to hidden window.
  ContentWindow::DidSetFocus(last_focused);
  ui::TextInputClient::Get()->set_delegate(this);
  RequestAnimationFrame();
}

void TextWindow::DidShow() {
  ContentWindow::DidShow();
  vertical_scroll_bar_->Show();
}

HCURSOR TextWindow::GetCursorAt(const gfx::Point&) const {
  return ::LoadCursor(nullptr, IDC_IBEAM);
}

// views::ContentWindow
void TextWindow::MakeSelectionVisible() {
  // Redraw() will format text buffer to place caret on center of screen.
  caret_offset_ = -1;
  RequestAnimationFrame();
}

}  // namespace views
