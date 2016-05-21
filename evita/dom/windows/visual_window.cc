// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/windows/visual_window.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/trace_event/trace_event.h"
#include "evita/dom/bindings/exception_state.h"
#include "evita/dom/css/css_style_sheet_handle.h"
#include "evita/dom/public/cursor.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/public/view_events.h"
#include "evita/dom/scheduler/animation_frame_callback.h"
#include "evita/dom/scheduler/scheduler.h"
#include "evita/dom/script_host.h"
#include "evita/dom/visuals/node_handle.h"
#include "evita/visuals/css/media_state.h"
#include "evita/visuals/css/media_type.h"
#include "evita/visuals/css/style_sheet.h"
#include "evita/visuals/display/public/display_item_list.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/view/public/selection.h"
#include "evita/visuals/view/view.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// VisualWindow
//
VisualWindow::VisualWindow(ScriptHost* script_host,
                           visuals::Document* document,
                           visuals::css::StyleSheet* style_sheet)
    : Scriptable(script_host),
      view_(new visuals::View(*document, *this, *this, {style_sheet})) {
  script_host->view_delegate()->CreateVisualWindow(window_id());
  view_->AddObserver(this);
}

VisualWindow::~VisualWindow() {}

const visuals::Document& VisualWindow::document() const {
  return view_->document();
}

void VisualWindow::CancelAnimationFrame() {
  if (animation_request_id_ == 0)
    return;
  script_host()->scheduler()->CancelAnimationFrame(animation_request_id_);
  animation_request_id_ = 0;
}

void VisualWindow::DidBeginAnimationFrame(const base::TimeTicks& now) {
  DCHECK_NE(animation_request_id_, 0);
  animation_request_id_ = 0;
  if (!visible())
    return;
  TRACE_EVENT_WITH_FLOW0("visuals", "VisualWindow::DidBeginAnimationFrame",
                         window_id(), TRACE_EVENT_FLAG_FLOW_OUT);
  auto display_item_list = view_->Paint();
  if (!display_item_list)
    return;
  script_host()->view_delegate()->PaintVisualDocument(
      window_id(), std::move(display_item_list));
}

void VisualWindow::RequestAnimationFrame() {
  if (!visible() || viewport_size_.IsEmpty())
    return;
  if (animation_request_id_)
    return;
  auto callback = std::make_unique<AnimationFrameCallback>(
      FROM_HERE, base::Bind(&VisualWindow::DidBeginAnimationFrame,
                            base::Unretained(this)));
  animation_request_id_ =
      script_host()->scheduler()->RequestAnimationFrame(std::move(callback));
}

// Binding callbacks
int VisualWindow::HitTest(int x, int y) {
  gfx::FloatPoint point(x, y);
  const auto& found = view_->HitTest(point);
  if (!found)
    return -1;
  return found.node()->sequence_id();
}

VisualWindow* VisualWindow::NewWindow(ScriptHost* script_host,
                                      NodeHandle* document_handle,
                                      CSSStyleSheetHandle* style_sheet_handle,
                                      ExceptionState* exception_state) {
  const auto document = document_handle->value()->as<visuals::Document>();
  if (!document) {
    exception_state->ThrowError("Requires document node");
    return nullptr;
  }
  return new VisualWindow(script_host, document, style_sheet_handle->value());
}

// ViewEventTarget
bool VisualWindow::HandleMouseEvent(const domapi::MouseEvent& event) {
  if (event.event_type != domapi::EventType::MouseMove)
    return false;
  script_host()->view_delegate()->SetCursor(window_id(),
                                            domapi::CursorId::Pointer);
  const auto& point = gfx::FloatPoint(event.client_x, event.client_y);
  const auto& found = view_->HitTest(point);
  if (hovered_node_ == found.node())
    return false;
  hovered_node_ = found.node();
  FOR_EACH_OBSERVER(UserActionSource::Observer, observers_,
                    DidChangeHoveredNode(hovered_node_));
  return false;
}

// visuals::css::Media
visuals::css::MediaState VisualWindow::media_state() const {
  return visuals::css::MediaState::Interactive;
}

visuals::css::MediaType VisualWindow::media_type() const {
  return visuals::css::MediaType::Screen;
}

gfx::FloatSize VisualWindow::viewport_size() const {
  return viewport_size_;
}

// visuals::UserActionSource
void VisualWindow::AddObserver(UserActionSource::Observer* observer) const {
  observers_.AddObserver(observer);
}

void VisualWindow::RemoveObserver(UserActionSource::Observer* observer) const {
  observers_.RemoveObserver(observer);
}

// visuals::ViewObserver
void VisualWindow::DidChangeView() {
  RequestAnimationFrame();
}

// Window
void VisualWindow::DidChangeBounds(int left, int top, int right, int bottom) {
  Window::DidChangeBounds(left, top, right, bottom);
  viewport_size_ = gfx::FloatSize(right - left, bottom - top);
  visuals::css::Media::DidChangeViewportSize();
}

void VisualWindow::DidDestroyWindow() {
  Window::DidDestroyWindow();
  view_->RemoveObserver(this);
  CancelAnimationFrame();
}

void VisualWindow::DidShowWindow() {
  Window::DidShowWindow();
  view_->ScheduleForcePaint();
  RequestAnimationFrame();
}

}  // namespace dom
