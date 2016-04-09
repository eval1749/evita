// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/bitmap.h"

#include "evita/gfx/canvas.h"

namespace gfx {

namespace {

common::ComPtr<ID2D1Bitmap> CreateBitmap(Canvas* canvas, SizeU size) {
  common::ComPtr<ID2D1Bitmap> bitmap;
  D2D1_BITMAP_PROPERTIES props;
  props.pixelFormat = (*canvas)->GetPixelFormat();
  (*canvas)->GetDpi(&props.dpiX, &props.dpiY);
  COM_VERIFY((*canvas)->CreateBitmap(size, props, &bitmap));
  return std::move(bitmap);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Bitmap
//
Bitmap::Bitmap(Canvas* canvas, common::ComPtr<ID2D1Bitmap> bitmap)
    : SimpleObject_(bitmap) {}

Bitmap::Bitmap(Canvas* canvas, SizeU size)
    : SimpleObject_(CreateBitmap(canvas, size)) {}

// TODO(eval1749): We should not u
Bitmap::Bitmap(Canvas* canvas)
    : Bitmap(canvas, SizeU((*canvas)->GetPixelSize())) {}

Bitmap::~Bitmap() {}

}  // namespace gfx
