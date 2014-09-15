// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/content_window.h"

#include "evita/editor/application.h"
#include "evita/gfx/canvas.h"
#include "evita/ui/compositor/layer.h"
#include "evita/views/content_observer.h"

namespace views {

ContentWindow::ContentWindow(views::WindowId window_id)
    : Window(window_id) {
}

ContentWindow::~ContentWindow() {
}

void ContentWindow::Activate() {
  #if DEBUG_FOCUS
    DVLOG(0) << "Activate " << this << " focus=" << has_focus() <<
        "show=" << visible();
  #endif
  RequestFocus();
}

void ContentWindow::AddObserver(ContentObserver* observer) {
  observers_.AddObserver(observer);
}

void ContentWindow::NotifyUpdateContent() {
  FOR_EACH_OBSERVER(ContentObserver, observers_, DidUpdateContent(this));
}

void ContentWindow::RemoveObserver(ContentObserver* observer) {
  observers_.RemoveObserver(observer);
}

// ui::LayerOwnerDelegate
void ContentWindow::DidRecreateLayer(ui::Layer*) {
  if (!canvas_)
    return;
  canvas_.reset(layer()->CreateCanvas());
}

// ui::Widget
void ContentWindow::DidChangeBounds() {
  Window::DidChangeBounds();
  if (!layer())
    return;
  layer()->SetBounds(bounds());
  if (canvas_)
    canvas_->SetBounds(GetContentsBounds());
}

void ContentWindow::DidChangeHierarchy() {
  Window::DidChangeHierarchy();
  container_widget().layer()->AppendLayer(layer());
}

void ContentWindow::DidHide() {
  Window::DidHide();
  canvas_.reset();
}

void ContentWindow::DidRealize() {
  Window::DidRealize();
  SetLayer(new ui::Layer());
  set_layer_owner_delegate(this);
}

void ContentWindow::DidSetFocus(ui::Widget* widget) {
  Window::DidSetFocus(widget);
  FOR_EACH_OBSERVER(ContentObserver, observers_, DidActivateContent(this));
}

void ContentWindow::DidShow() {
  Window::DidShow();
  DCHECK(!canvas_);
  if (bounds().empty())
    return;
  canvas_.reset(layer()->CreateCanvas());
}

}  // namespace views
