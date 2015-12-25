// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_BRUSH_H_
#define EVITA_GFX_BRUSH_H_

#include "evita/gfx/forward.h"
#include "evita/gfx/simple_object.h"

namespace gfx {

//////////////////////////////////////////////////////////////////////
//
// Brush
//
class Brush final : public SimpleObject_<ID2D1SolidColorBrush> {
 public:
  Brush(Canvas* canvas, ColorF color);
  Brush(Canvas* canvas, D2D1::ColorF::Enum name);
  Brush(Canvas* canvas, float red, float green, float blue, float alpha = 1.0f);
  ~Brush();

 private:
  DISALLOW_COPY_AND_ASSIGN(Brush);
};

}  // namespace gfx

#endif  // EVITA_GFX_BRUSH_H_
