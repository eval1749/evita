// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/block_box.h"

#include "evita/visuals/style/display.h"

namespace visuals {

BlockBox::BlockBox(const FloatRect& bounds, const FloatRect& content_bounds)
    : ContainerBox(Display::Block, bounds, content_bounds) {}

BlockBox::~BlockBox() {}

}  // namespace visuals
