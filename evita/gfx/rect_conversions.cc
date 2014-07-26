// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/rect_conversions.h"

#include <ctgmath>

namespace gfx {

// Returns the smallest Rect that encloses the given RectF.
Rect ToEnclosingRect(const RectF& rect) {
  return gfx::Rect(static_cast<int>(::floor(rect.left)),
                  static_cast<int>(::floor(rect.top)),
                  static_cast<int>(::ceil(rect.right)),
                  static_cast<int>(::ceil(rect.bottom)));
}

// Returns the largest Rect that is enclosed by the given RectF.
Rect ToEnclosedRect(const RectF& rect) {
  return gfx::Rect(static_cast<int>(::ceil(rect.left)),
                   static_cast<int>(::ceil(rect.top)),
                   static_cast<int>(::floor(rect.right)),
                   static_cast<int>(::floor(rect.bottom)));
}

}   // namespace gfx
