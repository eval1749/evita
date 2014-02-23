// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/point_f.h"

std::ostream& operator<<(std::ostream& ostream, const gfx::PointF& point) {
  return ostream << "(" << point.x << "," << point.y << ")";
}

std::ostream& operator<<(std::ostream& ostream, const gfx::PointF* point) {
  if (!point)
    return ostream << "(null)";
  return ostream <<*point;
}
