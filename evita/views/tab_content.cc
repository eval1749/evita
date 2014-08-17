// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/tab_content.h"

#include "common/win/native_window.h"
#include "evita/ui/compositor/layer.h"
#include "evita/vi_Frame.h"

namespace views {

TabContent::TabContent()
    : ui::Widget(), active_tick_(0) {
}

TabContent::~TabContent() {
}

void TabContent::Activate() {
  DEFINE_STATIC_LOCAL(int, static_active_tick, (0));
  ++static_active_tick;
  active_tick_ = static_active_tick;
  Show();
  RequestFocus();
}

Frame* TabContent::GetFrame() const {
  return container_widget().as<Frame>();
}

// ui::Widget
void TabContent::DidRealize() {
  SetLayer(new ui::Layer());
  ui::Widget::DidRealize();
}

}  // namespace views
