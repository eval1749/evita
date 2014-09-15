// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_gfx_bitmap_h)
#define INCLUDE_evita_gfx_bitmap_h

#include "evita/gfx_base.h"

namespace gfx {

class Bitmap : public SimpleObject_<ID2D1Bitmap> {
  public: Bitmap(Canvas* canvas, HICON hIcon);
  public: Bitmap(Canvas* canvas, SizeU size);
  public: explicit Bitmap(Canvas* canvas);
  public: ~Bitmap();
  DISALLOW_COPY_AND_ASSIGN(Bitmap);
};

} // namespace gfx

#endif //!defined(INCLUDE_evita_gfx_bitmap_h)
