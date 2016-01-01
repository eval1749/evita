// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/visuals/model/block_box.h"

namespace visuals {

BlockBox::BlockBox(const base::StringPiece16& id) : ContainerBox(id) {}

BlockBox::BlockBox() {}

BlockBox::~BlockBox() {}

FloatSize BlockBox::ComputePreferredSize() const {
  if (is_display_none())
    return FloatSize();
  auto size = border().size() + padding().size();
  for (const auto& child : child_boxes()) {
    if (!child->position().is_static())
      continue;
    const auto& child_size = child->ComputePreferredSize();
    size = FloatSize(std::max(size.width(), child_size.width()),
                     size.height() + child_size.height());
  }
  return size;
}

}  // namespace visuals
