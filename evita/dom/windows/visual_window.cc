// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/windows/visual_window.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/trace_event/trace_event.h"
#include "evita/dom/bindings/exception_state.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/scheduler.h"
#include "evita/dom/script_host.h"
#include "evita/dom/timing/animation_frame_callback.h"
#include "evita/dom/visuals/css_style_sheet_handle.h"
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
      view_(new visuals::View(*document, *this, {style_sheet})) {
  script_host->view_delegate()->CreateVisualWindow(window_id());
  view_->AddObserver(this);
}

VisualWindow::~VisualWindow() {
  view_->RemoveObserver(this);
}

const visuals::Document& VisualWindow::document() const {
  return view_->document();
}

void VisualWindow::DidBeginAnimationFrame(const base::TimeTicks& now) {
  TRACE_EVENT0("script", "VisualWindow::DidBeginAnimationFrame");
  DCHECK(is_waiting_animation_frame_);
  is_waiting_animation_frame_ = false;
  auto display_item_list = view_->Paint();
  if (!display_item_list)
    return;
  script_host()->view_delegate()->PaintVisualDocument(
      window_id(), std::move(display_item_list));
}

void VisualWindow::RequestAnimationFrame() {
  if (viewport_size_.IsEmpty())
    return;
  if (is_waiting_animation_frame_)
    return;
  is_waiting_animation_frame_ = true;
  auto callback = std::make_unique<AnimationFrameCallback>(
      FROM_HERE, base::Bind(&VisualWindow::DidBeginAnimationFrame,
                            base::Unretained(this)));
  script_host()->scheduler()->RequestAnimationFrame(std::move(callback));
}

// Binding callbacks
int VisualWindow::HitTest(int x, int y) {
  visuals::FloatPoint point(x, y);
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

// visuals::css::Media
visuals::css::MediaState VisualWindow::media_state() const {
  return visuals::css::MediaState::Interactive;
}

visuals::css::MediaType VisualWindow::media_type() const {
  return visuals::css::MediaType::Screen;
}

visuals::FloatSize VisualWindow::viewport_size() const {
  return viewport_size_;
}

// visuals::ViewObserver
void VisualWindow::DidChangeView() {
  RequestAnimationFrame();
}

// Window
void VisualWindow::DidChangeBounds(int left, int top, int right, int bottom) {
  viewport_size_ = visuals::FloatSize(right - left, bottom - top);
  visuals::css::Media::DidChangeViewportSize();
}

void VisualWindow::DidShowWindow() {
  view_->ScheduleForcePaint();
  RequestAnimationFrame();
}

}  // namespace dom
