// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/public/float_rect.h"

#include "evita/dom/public/float_point.h"

namespace domapi {

FloatRect::FloatRect(float x, float y, float width, float height)
    : height_(height), width_(width), x_(x), y_(y) {
}

FloatRect::FloatRect() : FloatRect(0.0f, 0.0f, 0.0f, 0.0f) {
}

FloatRect::~FloatRect() {
}

FloatPoint FloatRect::origin() const {
  return FloatPoint(x_, y_);
}

FloatPoint FloatRect::right_bottom() const {
  return FloatPoint(x_ + width_, y_ + height_);
}

}  // namespace domapi
