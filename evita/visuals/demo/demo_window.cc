// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/demo/demo_window.h"

#include "base/message_loop/message_loop.h"
#include "base/time/time.h"
#include "build/build_config.h"
#include "evita/gfx/base/geometry/float_rect.h"
#include "evita/gfx/brush.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/color_f.h"
#include "evita/ui/compositor/root_layer.h"
#include "evita/ui/events/event.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// WindowEventHandler
//
WindowEventHandler::WindowEventHandler() {}
WindowEventHandler::~WindowEventHandler() {}

//////////////////////////////////////////////////////////////////////
//
// DemoWindow
//
DemoWindow::DemoWindow(WindowEventHandler* event_handler,
                       const base::Closure& quit_closure)
    : ui::AnimatableWindow(ui::NativeWindow::Create(this)),
      event_handler_(event_handler),
      quit_closure_(quit_closure) {}

DemoWindow::~DemoWindow() {
  base::MessageLoop::current()->task_runner()->PostTask(FROM_HERE,
                                                        quit_closure_);
}

gfx::Canvas* DemoWindow::GetCanvas() const {
  if (!canvas_->UpdateReadyState())
    return nullptr;
  return canvas_.get();
}

// ui::AnimationFrameHandler
void DemoWindow::DidBeginAnimationFrame(const base::TimeTicks& time) {}

// ui::LayerOwnerDelegate
void DemoWindow::DidRecreateLayer(ui::Layer*) {
  if (!canvas_)
    return;
  canvas_.reset(layer()->CreateCanvas());
}

// ui::Widget
void DemoWindow::DidChangeBounds() {
  ui::Widget::DidChangeBounds();
  if (!canvas_)
    return;
  canvas_->SetBounds(GetContentsBounds());
  const auto& bounds = GetContentsBounds();
  event_handler_->DidChangeWindowBounds(
      gfx::FloatRect(gfx::FloatSize(bounds.width(), bounds.height())));
}

void DemoWindow::DidHide() {
  ui::Widget::DidHide();
  canvas_.reset();
}

void DemoWindow::DidKillFocus(ui::Widget* focused_window) {
  event_handler_->DidKillFocus();
}

void DemoWindow::DidRealize() {
  SetLayer(new ui::RootLayer(this));
  set_layer_owner_delegate(this);
  ui::Widget::DidRealize();
}

void DemoWindow::DidSetFocus(ui::Widget* last_focused) {
  event_handler_->DidSetFocus();
}

void DemoWindow::DidShow() {
  DCHECK(!canvas_);
  ui::Widget::DidShow();
  if (bounds().empty())
    return;
  canvas_.reset(layer()->CreateCanvas());
}

void DemoWindow::OnKeyPressed(const ui::KeyEvent& event) {
  event_handler_->DidPressKey(event.raw_key_code());
}

void DemoWindow::OnMouseMoved(const ui::MouseEvent& event) {
  event_handler_->DidMoveMouse(
      gfx::FloatPoint(event.location().x(), event.location().y()));
}

void DemoWindow::OnMousePressed(const ui::MouseEvent& event) {
  event_handler_->DidPressMouse(
      gfx::FloatPoint(event.location().x(), event.location().y()));
}

}  // namespace visuals
