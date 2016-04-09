// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <utility>

#include "evita/visuals/dom/image_data.h"

#include "base/strings/utf_string_conversions.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ImageData
//
ImageData::ImageData(const ImageBitmap& bitmap)
    : bitmap_(bitmap), bounds_(bitmap.size()) {}

ImageData::ImageData(const ImageData& other)
    : bitmap_(other.bitmap_), bounds_(other.bounds_) {}

ImageData::ImageData(ImageData&& other)
    : bitmap_(std::move(other.bitmap_)), bounds_(other.bounds_) {}

ImageData::~ImageData() {}

ImageData& ImageData::operator=(const ImageData& other) {
  bitmap_ = other.bitmap_;
  bounds_ = other.bounds_;
  return *this;
}

ImageData& ImageData::operator=(ImageData&& other) {
  bitmap_ = other.bitmap_;
  bounds_ = other.bounds_;
  return *this;
}

bool ImageData::operator==(const ImageData& other) const {
  if (this == &other)
    return true;
  return bitmap_ == other.bitmap_ && bounds_ == other.bounds_;
}

bool ImageData::operator!=(const ImageData& other) const {
  return !operator==(other);
}

std::ostream& operator<<(std::ostream& ostream, const ImageData& data) {
  return ostream << "ImageData(" << data.bitmap() << ' ' << data.bounds()
                 << ')';
}

}  // namespace visuals
