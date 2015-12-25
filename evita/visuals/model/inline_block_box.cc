// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/inline_block_box.h"

#include "evita/visuals/style/display.h"

namespace visuals {

InlineBlockBox::InlineBlockBox(const FloatRect& bounds,
                               const FloatRect& content_bounds)
    : ContainerBox(Display::InlineBlock, bounds, content_bounds) {}

InlineBlockBox::~InlineBlockBox() {}

}  // namespace visuals
