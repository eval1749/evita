// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_gfx_rect_conversions_h)
#define INCLUDE_gfx_rect_conversions_h

#include "evita/gfx/rect.h"
#include "evita/gfx/rect_f.h"

namespace gfx {

// Returns the smallest Rect that encloses the given RectF.
Rect ToEnclosingRect(const RectF& rect);

// Returns the largest Rect that is enclosed by the given RectF.
Rect ToEnclosedRect(const RectF& rect);

}  // namespace gfx

#endif //!defined(INCLUDE_gfx_rect_conversions_h)
