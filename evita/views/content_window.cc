// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/content_window.h"

#include "evita/gfx/canvas.h"
#include "evita/ui/compositor/layer.h"
#include "evita/views/content_observer.h"

namespace views {

ContentWindow::ContentWindow(views::WindowId window_id) : Window(window_id) {}

ContentWindow::~ContentWindow() {}

void ContentWindow::Activate() {
#if DEBUG_FOCUS
  DVLOG(0) << "Activate " << this << " focus=" << has_focus()
           << "show=" << visible();
#endif
  RequestFocus();
}

void ContentWindow::AddObserver(ContentObserver* observer) {
  observers_.AddObserver(observer);
}

void ContentWindow::NotifyUpdateContent() {
  for (auto& observer : observers_)
    observer.DidUpdateContent(this);
}

void ContentWindow::RemoveObserver(ContentObserver* observer) {
  observers_.RemoveObserver(observer);
}

// ui::Widget
void ContentWindow::DidChangeHierarchy() {
  Window::DidChangeHierarchy();
  container_widget()->layer()->AppendLayer(layer());
}

void ContentWindow::DidRealize() {
  Window::DidRealize();
  SetLayer(new ui::Layer());
  set_layer_owner_delegate(this);
}

void ContentWindow::DidSetFocus(ui::Widget* widget) {
  Window::DidSetFocus(widget);
  for (auto& observer : observers_)
    observer.DidActivateContent(this);
}

}  // namespace views
