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

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// DemoWindow
//
DemoWindow::DemoWindow(const base::Closure& quit_closure)
    : ui::AnimatableWindow(ui::NativeWindow::Create(this)),
      quit_closure_(quit_closure) {}

DemoWindow::~DemoWindow() {
  base::MessageLoop::current()->task_runner()->PostTask(FROM_HERE,
                                                        quit_closure_);
}

// ui::AnimationFrameHandler
void DemoWindow::DidBeginAnimationFrame(base::Time time) {
  if (!canvas_->IsReady())
    return RequestAnimationFrame();
  gfx::Canvas::DrawingScope scope(canvas_.get());
  canvas_->AddDirtyRect(GetContentsBounds());
  gfx::Brush edge_brush(canvas_.get(), gfx::ColorF(0.5f, 0.5f, 0.5f));
  canvas_->FillRectangle(edge_brush, GetContentsBounds());
}

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

}  // namespace visuals
