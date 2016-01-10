// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/demo/demo_window.h"

#include "base/time/time.h"
#include "base/message_loop/message_loop.h"
#include "build/build_config.h"
#include "evita/gfx/brush.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/color_f.h"
#include "evita/ui/compositor/root_layer.h"
#include "evita/ui/events/event.h"
#include "evita/visuals/geometry/float_rect.h"

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
  if (!canvas_->IsReady())
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
      FloatRect(FloatSize(bounds.width(), bounds.height())));
}

void DemoWindow::DidHide() {
  ui::Widget::DidHide();
  canvas_.reset();
}

void DemoWindow::DidRealize() {
  SetLayer(new ui::RootLayer(this));
  set_layer_owner_delegate(this);
  ui::Widget::DidRealize();
}

void DemoWindow::DidShow() {
  DCHECK(!canvas_);
  ui::Widget::DidShow();
  if (bounds().empty())
    return;
  canvas_.reset(layer()->CreateCanvas());
}

void DemoWindow::OnMouseMoved(const ui::MouseEvent& event) {
  event_handler_->DidMoveMouse(
      FloatPoint(event.location().x(), event.location().y()));
}

void DemoWindow::OnMousePressed(const ui::MouseEvent& event) {
  event_handler_->DidPressMouse(
      FloatPoint(event.location().x(), event.location().y()));
}

}  // namespace visuals
