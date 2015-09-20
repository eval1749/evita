// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/public/tab_data.h"

namespace domapi {

TabData::TabData() : icon(-1), state(State::Normal) {}

TabData::~TabData() {}

bool TabData::operator==(const TabData& other) const {
  return icon == other.icon && state == other.state && title == other.title &&
         tooltip == other.tooltip;
}

bool TabData::operator!=(const TabData& other) const {
  return !operator==(other);
}

}  // namespace domapi
