// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_RECT_CONVERSIONS_H_
#define EVITA_GFX_RECT_CONVERSIONS_H_

#include "evita/gfx/rect.h"
#include "evita/gfx/rect_f.h"

namespace gfx {

// Returns the largest Rect that is enclosed by the given RectF.
Rect ToEnclosedRect(const RectF& rect);

// Returns the smallest Rect that encloses the given RectF.
Rect ToEnclosingRect(const RectF& rect);

}  // namespace gfx

#endif  // EVITA_GFX_RECT_CONVERSIONS_H_
