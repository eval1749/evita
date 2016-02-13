// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/imaging/image_bitmap.h"

#include "build/build_config.h"

#if OS_WIN
#include "evita/visuals/imaging/native_image_bitmap_win.h"
#else
#error "Unsupported target"
#endif

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ImageBitmap
//
ImageBitmap::ImageBitmap(std::unique_ptr<NativeImageBitmap> impl)
    : impl_(std::move(impl)) {}
ImageBitmap::~ImageBitmap() {}

const FloatSize& ImageBitmap::size() const {
  return impl_->size();
}

}  // namespace visuals
