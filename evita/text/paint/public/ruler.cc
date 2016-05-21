// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/paint/public/ruler.h"

namespace paint {

Ruler::Ruler(float x_point, float size, const gfx::ColorF& color)
    : color_(color), size_(size), x_point_(x_point) {}

Ruler::Ruler(const Ruler& other)
    : Ruler(other.x_point_, other.size_, other.color_) {}

Ruler::~Ruler() = default;

}  // namespace paint
