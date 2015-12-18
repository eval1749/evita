// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/paint/public/selection.h"

namespace paint {

Selection::Selection(const gfx::ColorF color,
                     const std::unordered_set<gfx::RectF> bounds_set)
    : bounds_set_(bounds_set), color_(color) {}

Selection::~Selection() {}

bool Selection::HasBounds(const gfx::RectF& bounds) const {
  return bounds_set_.find(bounds) != bounds_set_.end();
}

}  // namespace paint
