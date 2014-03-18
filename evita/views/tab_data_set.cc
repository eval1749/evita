// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/tab_data_set.h"

#include "evita/dom/public/tab_data.h"

namespace views {

TabDataSet::TabDataSet() {
}

TabDataSet::~TabDataSet() {
}

const TabDataSet::TabData* TabDataSet::GetTabData(dom::WindowId window_id) {
  auto const it = map_.find(window_id);
  return it == map_.end() ? nullptr : it->second;
}

void TabDataSet::RemoveTabData(dom::WindowId window_id) {
  auto const it = map_.find(window_id);
  if (it == map_.end())
    return;
  delete it->second;
  map_.erase(it);
}

void TabDataSet::SetTabData(dom::WindowId window_id, const TabData& tab_data) {
  auto const it = map_.find(window_id);
  if (it == map_.end()) {
    map_[window_id] = new TabData(tab_data);
    return;
  }
  *it->second = tab_data;
}

}  // namespace views
