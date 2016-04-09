// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_IMAGE_DATA_H_
#define EVITA_VISUALS_DOM_IMAGE_DATA_H_

#include <stdint.h>

#include <iosfwd>
#include <memory>

#include "base/macros.h"
#include "evita/gfx/base/geometry/float_rect.h"
#include "evita/visuals/imaging/image_bitmap.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ImageData
//
class ImageData final {
 public:
  explicit ImageData(const ImageBitmap& bitmap);
  ImageData(const ImageData& other);
  ImageData(ImageData&& other);
  ~ImageData();

  ImageData& operator=(const ImageData& other);
  ImageData& operator=(ImageData&& other);

  bool operator==(const ImageData& other) const;
  bool operator!=(const ImageData& other) const;

  const ImageBitmap& bitmap() const { return bitmap_; }
  const gfx::FloatRect& bounds() const { return bounds_; }

 private:
  ImageBitmap bitmap_;
  // Image fragment bounds specified by image URL "#xywh=\d+,\d+,\d+,\d"
  gfx::FloatRect bounds_;
};

std::ostream& operator<<(std::ostream& ostream, const ImageData& data);

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_IMAGE_DATA_H_
