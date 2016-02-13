// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/imaging/native_image_bitmap_win.h"

#include "common/win/com_verify.h"
#include "evita/gfx/imaging_factory_win.h"

namespace visuals {

namespace {

FloatSize ComputeSize(const base::win::ScopedComPtr<IWICBitmap>& bitmap) {
  uint32_t width = 0;
  uint32_t height = 0;
  COM_VERIFY(bitmap->GetSize(&width, &height));
  return FloatSize(static_cast<float>(width), static_cast<float>(height));
}

base::win::ScopedComPtr<IWICBitmap> CreateBitmapFromIcon(
    const base::win::ScopedHICON& icon) {
  base::win::ScopedComPtr<IWICBitmap> bitmap;
  COM_VERIFY(gfx::ImagingFactory::GetInstance()->impl()->CreateBitmapFromHICON(
      icon.get(), bitmap.Receive()));
  return bitmap;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// NativeImageBitmap
//
NativeImageBitmap::NativeImageBitmap(const base::win::ScopedHICON& icon)
    : data_(CreateBitmapFromIcon(icon)), size_(ComputeSize(data_)) {}

NativeImageBitmap::~NativeImageBitmap() {}

}  // namespace visuals
