// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/canvas_content_window.h"

#include "evita/gfx/brush.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/color_f.h"
#include "evita/ui/compositor/layer.h"
#include "evita/views/content_observer.h"

namespace views {

CanvasContentWindow::CanvasContentWindow(views::WindowId window_id)
    : ContentWindow(window_id) {}

CanvasContentWindow::~CanvasContentWindow() {}

// ui::LayerOwnerDelegate
void CanvasContentWindow::DidRecreateLayer(ui::Layer*) {
  if (!canvas_)
    return;
  canvas_.reset(layer()->CreateCanvas());
}

// ui::Widget
void CanvasContentWindow::DidChangeBounds() {
  ContentWindow::DidChangeBounds();
  if (!canvas_)
    return;
  canvas_->SetBounds(GetContentsBounds());
}

void CanvasContentWindow::DidHide() {
  ContentWindow::DidHide();
  canvas_.reset();
}

void CanvasContentWindow::DidShow() {
  ContentWindow::DidShow();
  DCHECK(!canvas_);
  if (bounds().empty())
    return;
  canvas_.reset(layer()->CreateCanvas());
}

void CanvasContentWindow::OnDraw(gfx::Canvas* canvas) {
  ContentWindow::OnDraw(canvas);
  gfx::Brush edge_brush(canvas, gfx::ColorF(0, 0, 0, 0.1f));
  canvas->DrawRectangle(edge_brush, GetContentsBounds());
}

}  // namespace views
