// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/rect_f.h"

std::ostream& operator<<(std::ostream& ostream, const gfx::RectF& rect) {
  return ostream << rect.left_top() << "+" << rect.size();
}

std::ostream& operator<<(std::ostream& ostream, const gfx::RectF* rect) {
  if (!rect)
    return ostream << "(null)";
  return ostream <<*rect;
}

namespace std {
size_t hash<gfx::RectF>::operator()(const gfx::RectF& rect) const {
  auto result = std::hash<float>()(rect.bottom);
  result ^= std::hash<float>()(rect.left);
  result ^= std::hash<float>()(rect.right);
  result ^= std::hash<float>()(rect.top);
  return result;
}
}  // namespace std
