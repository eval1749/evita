// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/tab_content.h"

#include "common/win/native_window.h"
#include "evita/ui/compositor/layer.h"
#include "evita/views/tab_content_observer.h"
#include "evita/vi_Frame.h"

namespace views {

TabContent::TabContent()
    : ui::AnimatableWindow(), active_tick_(0) {
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

void TabContent::DidAnimateTabContent() {
  FOR_EACH_OBSERVER(TabContentObserver, observers_, DidAnimateTabContent(this));
}

void TabContent::AddObserver(TabContentObserver* observer) {
  observers_.AddObserver(observer);
}

Frame* TabContent::GetFrame() const {
  return container_widget().as<Frame>();
}

void TabContent::RemoveObserver(TabContentObserver* observer) {
  observers_.RemoveObserver(observer);
}

// ui::AnimatableWindow
void TabContent::Animate(base::Time) {
}

// ui::Widget
void TabContent::DidHide() {
  if (auto const parent_layer = layer()->parent_layer())
    parent_layer->RemoveChildLayer(layer());
}

void TabContent::DidRealize() {
  SetLayer(new ui::Layer());
  ui::AnimatableWindow::DidRealize();
}

}  // namespace views
