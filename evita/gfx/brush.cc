// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/brush.h"

#include "evita/gfx/canvas.h"
#include "evita/gfx/color_f.h"

namespace gfx {

namespace {
common::ComPtr<ID2D1SolidColorBrush> CreateSolidColorBrush(Canvas* canvas,
                                                           ColorF color) {
  common::ComPtr<ID2D1SolidColorBrush> brush;
  COM_VERIFY((*canvas)->CreateSolidColorBrush(color, &brush));
  return brush;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Brush
//
Brush::Brush(Canvas* canvas, ColorF color)
    : SimpleObject_(CreateSolidColorBrush(canvas, color)) {}

Brush::Brush(Canvas* canvas, D2D1::ColorF::Enum name)
    : Brush(canvas, ColorF(name)) {}

Brush::Brush(Canvas* canvas, float red, float green, float blue, float alpha)
    : SimpleObject_(
          CreateSolidColorBrush(canvas, ColorF(red, green, blue, alpha))) {}

Brush::~Brush() {
#if _DEBUG
  auto const ref_count = (*this)->AddRef();
  DCHECK_EQ(2u, ref_count);
  (*this)->Release();
#endif
}

}  // namespace gfx
