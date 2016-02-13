// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_IMAGING_IMAGE_BITMAP_H_
#define EVITA_VISUALS_IMAGING_IMAGE_BITMAP_H_

#include <memory>

#include "base/macros.h"
#include "evita/visuals/geometry/float_size.h"

namespace visuals {

class NativeImageBitmap;

//////////////////////////////////////////////////////////////////////
//
// ImageBitmap
//
class ImageBitmap final {
 public:
  explicit ImageBitmap(std::unique_ptr<NativeImageBitmap> native_image);
  ~ImageBitmap();

  const NativeImageBitmap& impl() const { return *impl_; }
  const FloatSize& size() const;

 private:
  std::unique_ptr<NativeImageBitmap> impl_;

  DISALLOW_COPY_AND_ASSIGN(ImageBitmap);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_IMAGING_IMAGE_BITMAP_H_
