// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/bitmap.h"

namespace gfx {

namespace {

common::ComPtr<ID2D1Bitmap> CreateBitmap(const Canvas& gfx, HICON hIcon) {
  common::ComPtr<IWICBitmap> icon;
  COM_VERIFY(gfx::FactorySet::image().CreateBitmapFromHICON(
      hIcon, &icon));
 common::ComPtr<IWICFormatConverter> converter;
 COM_VERIFY(gfx::FactorySet::image().
    CreateFormatConverter(&converter));
 COM_VERIFY(converter->Initialize(
      icon,
      GUID_WICPixelFormat32bppPBGRA,
      WICBitmapDitherTypeNone,
      nullptr,
      0,
      WICBitmapPaletteTypeMedianCut));
  common::ComPtr<ID2D1Bitmap> bitmap;
  COM_VERIFY(gfx->CreateBitmapFromWicBitmap(converter, nullptr, &bitmap));
  return std::move(bitmap);
}

common::ComPtr<ID2D1Bitmap> CreateBitmap(const Canvas& gfx, SizeU size) {
  common::ComPtr<ID2D1Bitmap> bitmap;
  D2D1_BITMAP_PROPERTIES props;
  props.pixelFormat = gfx->GetPixelFormat();
  gfx->GetDpi(&props.dpiX, &props.dpiY);
  COM_VERIFY(gfx->CreateBitmap(size, props, &bitmap));
  return std::move(bitmap);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Bitmap
//
Bitmap::Bitmap(const Canvas& gfx, HICON hIcon)
    : SimpleObject_(CreateBitmap(gfx, hIcon)) {
}

Bitmap::Bitmap(const Canvas& gfx, SizeU size)
    : SimpleObject_(CreateBitmap(gfx, size)) {
}

Bitmap::Bitmap(const Canvas& gfx)
    : Bitmap(gfx, gfx->GetPixelSize()) {
}

Bitmap::~Bitmap() {
}

}  // namespace gfx
