// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/visuals/model/line_box.h"

#include "evita/visuals/style/display.h"

namespace visuals {

LineBox::LineBox() {}

LineBox::~LineBox() {}

FloatSize LineBox::ComputePreferredSize() const {
  if (display() == css::Display::None)
    return FloatSize();
  auto size = border().size() + padding().size();
  for (const auto& child : child_boxes()) {
    const auto& child_size = child->ComputePreferredSize();
    size = FloatSize(size.width() + child_size.width(),
                     std::max(size.height(), child_size.height()));
  }
  return size;
}

}  // namespace visuals
