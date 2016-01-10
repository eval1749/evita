// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/windows/visual_window.h"

#include "base/logging.h"
#include "base/trace_event/trace_event.h"
#include "evita/dom/scheduler.h"
#include "evita/dom/script_host.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/timing/animation_frame_callback.h"
#include "evita/dom/visuals/node_handle.h"
#include "evita/visuals/css/media_type.h"
#include "evita/visuals/css/style_sheet.h"
#include "evita/visuals/display/public/display_item_list.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/layout/box_tree.h"
#include "evita/visuals/layout/layouter.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/paint/painter.h"
#include "evita/visuals/paint/paint_info.h"
#include "evita/visuals/style/style_tree.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// VisualWindow
//
VisualWindow::VisualWindow(visuals::Document* document)
    : style_sheet_(new CssStyleSheet()),
      style_tree_(
          new visuals::StyleTree(*document, *this, {style_sheet_.get()})),
      box_tree_(new visuals::BoxTree(*document, *style_tree_)) {
  ScriptHost::instance()->view_delegate()->CreateVisualWindow(window_id());
  document->AddObserver(this);
}

VisualWindow::~VisualWindow() {
  style_tree_->document().RemoveObserver(this);
}

void VisualWindow::DidBeginAnimationFrame(const base::Time& now) {
  TRACE_EVENT0("script", "VisualWindow::DidBeginAnimationFrame");
  DCHECK(is_waiting_animation_frame_);
  is_waiting_animation_frame_ = false;
  UpdateStyleIfNeeded();
  UpdateLayoutIfNeeded();
  visuals::PaintInfo paint_info(box_tree_->root_box()->bounds(),
                                base::string16());
  auto display_item_list =
      visuals::Painter().Paint(paint_info, *box_tree_->root_box());
  ScriptHost::instance()->view_delegate()->PaintVisualDocument(
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
  ScriptHost::instance()->scheduler()->RequestAnimationFrame(
      std::move(callback));
}

void VisualWindow::UpdateLayoutIfNeeded() {
  UpdateStyleIfNeeded();
  box_tree_->UpdateIfNeeded();
  visuals::Layouter().Layout(box_tree_->root_box());
}

void VisualWindow::UpdateStyleIfNeeded() {
  style_tree_->UpdateIfNeeded();
}

// Binding callbacks
VisualWindow* VisualWindow::NewWindow(NodeHandle* document_handle) {
  const auto document = document_handle->value()->as<visuals::Document>();
  if (!document) {
    ScriptHost::instance()->ThrowError("Requires document node");
    return nullptr;
  }
  return new VisualWindow(document);
}

// visuals::css::Media
visuals::css::MediaType VisualWindow::media_type() const {
  return visuals::css::MediaType::Screen;
}

visuals::FloatSize VisualWindow::viewport_size() const {
  return viewport_size_;
}

// visuals::DocumentObserver
void VisualWindow::DidAddClass(const Element& element,
                               const base::string16& new_name) {
  RequestAnimationFrame();
}
void VisualWindow::DidAppendChild(const ContainerNode& parent,
                                  const Node& child) {
  RequestAnimationFrame();
}
void VisualWindow::DidChangeInlineStyle(const Element& element,
                                        const CssStyle* old_style) {
  RequestAnimationFrame();
}
void VisualWindow::DidInsertBefore(const ContainerNode& parent,
                                   const Node& child,
                                   const Node& ref_child) {
  RequestAnimationFrame();
}
void VisualWindow::DidRemoveChild(const ContainerNode& parent,
                                  const Node& child) {
  RequestAnimationFrame();
}
void VisualWindow::DidRemoveClass(const Element& element,
                                  const base::string16& old_name) {
  RequestAnimationFrame();
}
void VisualWindow::WillRemoveChild(const ContainerNode& parent,
                                   const Node& child) {
  RequestAnimationFrame();
}

// Window
void VisualWindow::DidChangeBounds(int left, int top, int right, int bottom) {
  viewport_size_ = FloatSize(right - left, bottom - top);
  CssMedia::DidChangeViewportSize();
  RequestAnimationFrame();
}

void VisualWindow::DidRealizeWindow() {
  Window::DidRealizeWindow();
  RequestAnimationFrame();
}

}  // namespace dom
