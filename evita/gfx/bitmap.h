// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_BITMAP_H_
#define EVITA_GFX_BITMAP_H_

#include "evita/gfx/simple_object.h"
#include "evita/gfx/size_f.h"

interface ID2D1Bitmap;

namespace gfx {

class Canvas;

//////////////////////////////////////////////////////////////////////
//
// Bitmap
//
class Bitmap final : public SimpleObject_<ID2D1Bitmap> {
 public:
  Bitmap(Canvas* canvas, HICON hIcon);
  Bitmap(Canvas* canvas, SizeU size);
  explicit Bitmap(Canvas* canvas);
  ~Bitmap();

 private:
  DISALLOW_COPY_AND_ASSIGN(Bitmap);
};

}  // namespace gfx

#endif  // EVITA_GFX_BITMAP_H_
