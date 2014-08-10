// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/vi_Pane.h"

#include "common/win/native_window.h"
#include "evita/ui/compositor/layer.h"
#include "evita/vi_Frame.h"

Pane::Pane()
    : ui::Widget(), active_tick_(0) {
}

Pane::~Pane() {
}

void Pane::Activate() {
  DEFINE_STATIC_LOCAL(int, static_active_tick, (0));
  ++static_active_tick;
  active_tick_ = static_active_tick;
  Show();
  RequestFocus();
}

Frame* Pane::GetFrame() const {
  return container_widget().as<Frame>();
}

// ui::Widget
void Pane::DidRealize() {
  SetLayer(new ui::Layer());
  ui::Widget::DidRealize();
}
