// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/dom/windows/text_window.h"

#include "base/logging.h"
#include "base/timer/timer.h"
#include "base/trace_event/trace_event.h"
#include "evita/dom/bindings/exception_state.h"
#include "evita/dom/bindings/ginx_TextDocumentEventInit.h"
#include "evita/dom/events/text_document_event.h"
#include "evita/dom/promise_resolver.h"
#include "evita/dom/public/scroll_bar_orientation.h"
#include "evita/dom/public/scroll_bar_part.h"
#include "evita/dom/public/text_area_display_item.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/scheduler.h"
#include "evita/dom/script_host.h"
#include "evita/dom/text/text_document.h"
#include "evita/dom/text/text_range.h"
#include "evita/dom/timing/animation_frame_callback.h"
#include "evita/dom/windows/scroll_bar.h"
#include "evita/dom/windows/text_selection.h"
#include "evita/layout/paint_view_builder.h"
#include "evita/layout/render_selection.h"
#include "evita/layout/text_view.h"
#include "evita/paint/public/caret.h"
#include "evita/paint/public/selection.h"
#include "evita/paint/public/view.h"
#include "evita/text/buffer.h"
#include "evita/text/selection.h"
#include "evita/ui/base/selection_state.h"
#include "evita/visuals/display/public/display_item_list.h"

namespace dom {

using Caret = TextWindow::Caret;
using CaretDisplayItem = paint::Caret;
using domapi::FloatPoint;
using domapi::FloatRect;
using domapi::FloatSize;
using domapi::ScrollBarData;
using layout::PaintViewBuilder;
using layout::TextSelectionModel;
using paint::CaretState;

namespace {

const auto kBlinkInterval = 16 * 20;  // milliseconds

FloatPoint ToFloatPoint(const gfx::PointF& point) {
  return FloatPoint(point.x, point.y);
}

FloatSize ToFloatSize(const gfx::SizeF& size) {
  return FloatSize(size.width, size.height);
}

FloatRect ToFloatRect(const gfx::RectF& rect) {
  return FloatRect(ToFloatPoint(rect.origin()), ToFloatSize(rect.size()));
}

gfx::RectF ToRectF(const FloatRect& rect) {
  return gfx::RectF(rect.x(), rect.y(), rect.right(), rect.bottom());
}

TextSelectionModel ComputeTextSelectionModel(TextWindow* window,
                                             const text::Selection& selection) {
  // TODO(eval1749): We should use |css::MediaState| instead of
  // |ui::SelectionState|.
  return TextSelectionModel(
      Window::GetFocusWindow() == window ? ui::SelectionState::HasFocus
                                         : ui::SelectionState::Disabled,
      selection.anchor_offset(), selection.focus_offset());
}

base::TimeDelta GetCaretBlinkInterval() {
  const auto interval = ::GetCaretBlinkTime();
  if (!interval)
    return base::TimeDelta::FromMilliseconds(kBlinkInterval);
  if (interval == INFINITE)
    return base::TimeDelta();
  return base::TimeDelta::FromMilliseconds(interval);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextWindow::Caret
//
class TextWindow::Caret final {
 public:
  explicit Caret(TextWindow* owner);
  ~Caret() = default;

  void DidChangeWindowBounds();
  void DidHideWindow();
  void DidKillFocus();
  void DidSetFocus();
  CaretDisplayItem Paint() const;
  void Update(const FloatRect& new_bounds, const base::TimeTicks& now);

 private:
  void DidFireCaretTimer();
  void StartCaretBlinkTimer();
  void StopCaretBlinkTimer();

  FloatRect bounds_;
  TextWindow* const owner_;
  CaretState state_ = CaretState::None;
  base::TimeTicks show_start_time_;
  base::RepeatingTimer blink_timer_;

  DISALLOW_COPY_AND_ASSIGN(Caret);
};

Caret::Caret(TextWindow* owner) : owner_(owner) {}

void Caret::DidFireCaretTimer() {
  owner_->RequestAnimationFrame();
}

void Caret::DidChangeWindowBounds() {
  StopCaretBlinkTimer();
}

void Caret::DidKillFocus() {
  StopCaretBlinkTimer();
}

void Caret::DidHideWindow() {
  StopCaretBlinkTimer();
}

void Caret::DidSetFocus() {
  DCHECK(state_ == CaretState::None);
}

CaretDisplayItem Caret::Paint() const {
  return CaretDisplayItem(state_, ToRectF(bounds_));
}

void Caret::StartCaretBlinkTimer() {
  const auto interval = GetCaretBlinkInterval();
  if (interval == base::TimeDelta())
    return;
  blink_timer_.Start(FROM_HERE, interval, base::Bind(&Caret::DidFireCaretTimer,
                                                     base::Unretained(this)));
}

void Caret::StopCaretBlinkTimer() {
  blink_timer_.Stop();
  bounds_ = FloatRect();
  state_ = CaretState::None;
}

void Caret::Update(const FloatRect& new_bounds, const base::TimeTicks& now) {
  if (bounds_ == new_bounds) {
    if (bounds_.IsEmpty()) {
      DCHECK(state_ == CaretState::None);
      return;
    }

    // When the caret stays at same point, caret is blinking.
    const auto interval = GetCaretBlinkInterval();
    if (interval == base::TimeDelta())
      return;
    const auto delta = now - show_start_time_;
    const auto index = delta / interval;
    state_ = index % 2 ? paint::CaretState::Hide : paint::CaretState::Show;
    return;
  }

  bounds_ = new_bounds;
  if (bounds_.IsEmpty()) {
    StopCaretBlinkTimer();
    state_ = CaretState::None;
    return;
  }
  state_ = CaretState::Show;
  show_start_time_ = now;
  StartCaretBlinkTimer();
}

//////////////////////////////////////////////////////////////////////
//
// TextWindow
//
TextWindow::TextWindow(ScriptHost* script_host, TextRange* selection_range)
    : Scriptable(script_host),
      caret_(new Caret(this)),
      selection_(new TextSelection(this, selection_range)),
      text_view_(new layout::TextView(*selection_range->document()->buffer())),
      vertical_scroll_bar_(
          new ScrollBar(domapi::ScrollBarOrientation::Vertical, this, this)) {
  document()->buffer()->AddObserver(this);
  selection_->text_selection()->AddObserver(this);
  script_host->view_delegate()->CreateTextWindow(window_id());
}

TextWindow::~TextWindow() {
  document()->buffer()->RemoveObserver(this);
  selection_->text_selection()->RemoveObserver(this);
}

text::Buffer* TextWindow::buffer() const {
  return document()->buffer();
}

TextDocument* TextWindow::document() const {
  return selection_->document();
}

float TextWindow::zoom() const {
  return text_view_->zoom();
}

void TextWindow::set_zoom(float new_zoom, ExceptionState* exception_state) {
  if (text_view_->zoom() == new_zoom)
    return;
  if (new_zoom <= 0.0f) {
    exception_state->ThrowRangeError(
        "TextWindow zoom must be greater than zero.");
    return;
  }
  text_view_->SetZoom(new_zoom);
  RequestAnimationFrame();
}

text::Offset TextWindow::ComputeMotion(int method,
                                       text::Offset position,
                                       int count,
                                       const FloatPoint& point) {
  domapi::TextWindowCompute data;
  data.method = static_cast<domapi::TextWindowCompute::Method>(method);
  data.count = count;
  data.position = position;
  data.x = point.x();
  data.y = point.y();
  switch (data.method) {
    case domapi::TextWindowCompute::Method::EndOfWindow:
      return ComputeWindowMotion(1, text::Offset(0));
    case domapi::TextWindowCompute::Method::EndOfWindowLine:
      return ComputeEndOfLine(data.position);
    case domapi::TextWindowCompute::Method::MoveScreen:
      return ComputeScreenMotion(data.count, point, data.position);
    case domapi::TextWindowCompute::Method::MoveWindow:
      return ComputeWindowMotion(data.count, data.position);
    case domapi::TextWindowCompute::Method::MoveWindowLine:
      return ComputeWindowLineMotion(data.count, point, data.position);
    case domapi::TextWindowCompute::Method::StartOfWindow:
      return ComputeWindowMotion(-1, text::Offset(0));
    case domapi::TextWindowCompute::Method::StartOfWindowLine:
      return ComputeStartOfLine(data.position);
    default:
      return text::Offset::Invalid();
  }
}

text::Offset TextWindow::ComputeMotion(int method,
                                       text::Offset position,
                                       int count) {
  return ComputeMotion(method, position, count, FloatPoint());
}

text::Offset TextWindow::ComputeMotion(int method, text::Offset position) {
  return ComputeMotion(method, position, 1, FloatPoint());
}

text::Offset TextWindow::ComputeMotion(int method) {
  return ComputeMotion(method, text::Offset(0), 1, FloatPoint());
}

text::Offset TextWindow::ComputeEndOfLine(text::Offset text_offset) {
  return text_view_->ComputeEndOfLine(text_offset);
}

text::Offset TextWindow::ComputeScreenMotion(int n,
                                             const FloatPoint& point,
                                             text::Offset offset) {
  // TODO(eval1749): We should not call |LargetScroll()| in |ComputeMotion|.
  if (LargeScroll(0, n))
    return HitTestPoint(point.x(), point.y());
  if (n > 0)
    return std::min(text_view_->text_end(), buffer()->GetEnd());
  if (n < 0)
    return text_view_->text_start();
  return offset;
}

text::Offset TextWindow::ComputeStartOfLine(text::Offset text_offset) {
  return text_view_->ComputeStartOfLine(text_offset);
}

text::Offset TextWindow::ComputeWindowLineMotion(int n,
                                                 const FloatPoint& pt,
                                                 text::Offset lPosn) {
  text_view_->FormatIfNeeded();
  if (n > 0) {
    const auto lBufEnd = buffer()->GetEnd();
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
    return text_view_->MapPointXToOffset(std::min(lGoal, lBufEnd), pt.x());
  }
  if (n < 0) {
    n = -n;
    const auto lBufStart = text::Offset(0);
    auto lStart = lPosn;
    auto k = 0;
    for (k = 0; k < n; ++k) {
      lStart = ComputeStartOfLine(lStart);
      if (lStart <= lBufStart)
        break;
      --lStart;
    }

    return text_view_->MapPointXToOffset(std::max(lStart, lBufStart), pt.x());
  }
  return lPosn;
}

text::Offset TextWindow::ComputeWindowMotion(int n, text::Offset offset) {
  text_view_->FormatIfNeeded();
  if (n > 0)
    return std::max(std::min(text_view_->text_end() - text::OffsetDelta(1),
                             buffer()->GetEnd()),
                    text_view_->text_start());
  if (n < 0)
    return text_view_->text_start();
  return offset;
}

void TextWindow::DidBeginAnimationFrame(const base::TimeTicks& now) {
  DCHECK(is_waiting_animation_frame_);
  is_waiting_animation_frame_ = false;
  if (!visible())
    return;
  TRACE_EVENT_WITH_FLOW0("view", "TextWindow::DidBeginAnimationFrame",
                         window_id(), TRACE_EVENT_FLAG_FLOW_OUT);
  const auto selection =
      ComputeTextSelectionModel(this, *selection_->text_selection());
  text_view_->Update(selection);
  const auto caret_bounds =
      ToFloatRect(text_view_->ComputeCaretBounds(selection));
  caret_->Update(caret_bounds, now);
  const auto paint_view =
      PaintViewBuilder().Build(text_view_->block(), selection, caret_->Paint());
  UpdateScrollBar();
  auto display_item = std::make_unique<domapi::TextAreaDisplayItem>(
      paint_view, std::move(vertical_scroll_bar_->Paint()));
  script_host()->view_delegate()->PaintTextArea(window_id(),
                                                std::move(display_item));
}

// Maps position specified buffer position and returns height
// of caret, If specified buffer position isn't in window, this function
// returns 0.
text::Offset TextWindow::HitTestPoint(float x, float y) {
  const auto& point = FloatPoint(x, y);
  const auto scroll_bar_part = vertical_scroll_bar_->HitTestPoint(point);
  if (scroll_bar_part != domapi::ScrollBarPart::None)
    return text::Offset::Invalid();
  text_view_->FormatIfNeeded();
  return std::min(text_view_->HitTestPoint(gfx::PointF(x, y)),
                  buffer()->GetEnd());
}

// Maps position specified buffer position and returns height
// of caret, If specified buffer position isn't in window, this function
// returns |text::Offset::Invalid()|.
domapi::FloatRect TextWindow::HitTestTextPosition(text::Offset offset) {
  text_view_->FormatIfNeeded();
  return ToFloatRect(text_view_->HitTestTextPosition(offset));
}

bool TextWindow::LargeScroll(int, int iDy) {
  text_view_->FormatIfNeeded();
  auto scrolled = false;
  if (iDy < 0) {
    // Scroll Down -- place top line out of window.
    iDy = -iDy;

    const auto lBufStart = text::Offset(0);
    for (auto k = 0; k < iDy; ++k) {
      const auto lStart = text_view_->text_start();
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
    const auto lBufEnd = buffer()->GetEnd();
    for (auto k = 0; k < iDy; ++k) {
      const auto lStart = text_view_->text_end();
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

void TextWindow::MakeSelectionVisible() {
  text_view_->MakeSelectionVisible();
}

// static
TextWindow* TextWindow::NewTextWindow(TextRange* selection_range) {
  return new TextWindow(script_host(), selection_range);
}

void TextWindow::Reconvert(const base::string16& text) {
  script_host()->view_delegate()->Reconvert(window_id(), text);
}

void TextWindow::RequestAnimationFrame() {
  if (is_waiting_animation_frame_)
    return;
  is_waiting_animation_frame_ = true;
  auto callback = std::make_unique<AnimationFrameCallback>(
      FROM_HERE,
      base::Bind(&TextWindow::DidBeginAnimationFrame, base::Unretained(this)));
  script_host()->scheduler()->RequestAnimationFrame(std::move(callback));
}

void TextWindow::Scroll(int direction) {
  SmallScroll(0, direction);
}

bool TextWindow::SmallScroll(int, int y_count) {
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
  DCHECK(!bounds().IsEmpty());
  const auto& canvas_bounds = bounds();

  const auto vertical_scroll_bar_width =
      static_cast<float>(::GetSystemMetrics(SM_CXVSCROLL));

  const auto text_block_bounds = FloatRect(
      canvas_bounds.size() - FloatSize(vertical_scroll_bar_width, 0.0f));
  text_view_->SetBounds(gfx::RectF(text_block_bounds.x(), text_block_bounds.y(),
                                   text_block_bounds.right(),
                                   text_block_bounds.bottom()));

  // Place vertical scroll bar at right edge of text block.
  const auto vertical_scroll_bar_bounds = FloatRect(
      text_block_bounds.top_right(),
      FloatSize(vertical_scroll_bar_width, text_block_bounds.height()));
  vertical_scroll_bar_->SetBounds(vertical_scroll_bar_bounds);
}

void TextWindow::UpdateScrollBar() {
  ScrollBarData data(base::FloatRange(0, buffer()->GetEnd().value() + 1),
                     base::FloatRange(text_view_->text_start().value(),
                                      text_view_->text_end().value()));
  vertical_scroll_bar_->SetData(data);
}

// text::BufferMutationObserver
void TextWindow::DidChangeStyle(const text::StaticRange& range) {
  text_view_->DidChangeStyle(range);
  RequestAnimationFrame();
}

void TextWindow::DidDeleteAt(const text::StaticRange& range) {
  text_view_->DidDeleteAt(range);
  RequestAnimationFrame();
}

void TextWindow::DidInsertBefore(const text::StaticRange& range) {
  text_view_->DidInsertBefore(range);
  RequestAnimationFrame();
}

// text::SelectionChangeObserver
void TextWindow::DidChangeSelection() {
  RequestAnimationFrame();
}

// ui::ScrollBarObserver
void TextWindow::DidClickLineDown() {
  SmallScroll(0, 1);
}

void TextWindow::DidClickLineUp() {
  SmallScroll(0, -1);
}

void TextWindow::DidClickPageDown() {
  LargeScroll(0, 1);
}

void TextWindow::DidClickPageUp() {
  LargeScroll(0, -1);
}

void TextWindow::DidMoveThumb(int value) {
  if (value < 0)
    return;
  text_view_->Format(text::Offset(value));
  RequestAnimationFrame();
}

// ViewEventTarget
bool TextWindow::HandleMouseEvent(const domapi::MouseEvent& event) {
  return vertical_scroll_bar_->HandleMouseEvent(event);
}

// Window
void TextWindow::DidChangeBounds() {
  UpdateBounds();
  RequestAnimationFrame();
  caret_->DidChangeWindowBounds();
}

void TextWindow::DidDestroyWindow() {
  // TODO(eval1749): We should dispatch "detach" event in JavaScript rather than
  // in C++;
  Window::DidDestroyWindow();
  TextDocumentEventInit init;
  init.set_bubbles(true);
  init.set_view(this);
  selection_->document()->ScheduleDispatchEventDeprecated(
      new TextDocumentEvent(L"detach", init));
}

void TextWindow::DidHideWindow() {
  Window::DidHideWindow();
  RequestAnimationFrame();
  caret_->DidHideWindow();
}

void TextWindow::DidKillFocus() {
  Window::DidKillFocus();
  RequestAnimationFrame();
  caret_->DidKillFocus();
}

void TextWindow::DidRealizeWindow() {
  // TODO(eval1749): We should dispatch "attach" event in JavaScript rather than
  // in C++;
  Window::DidRealizeWindow();
  TextDocumentEventInit init;
  init.set_bubbles(true);
  init.set_view(this);
  selection_->document()->ScheduleDispatchEventDeprecated(
      new TextDocumentEvent(L"attach", init));
}

void TextWindow::DidSetFocus() {
  Window::DidSetFocus();
  RequestAnimationFrame();
  caret_->DidSetFocus();
}

void TextWindow::DidShowWindow() {
  // Note: It is OK that hidden window have focus.
  Window::DidShowWindow();
  RequestAnimationFrame();
}

void TextWindow::ForceUpdateWindow() {
  RequestAnimationFrame();
}

}  // namespace dom
