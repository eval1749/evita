// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstring>
#include <ostream>
#include <utility>

#include "evita/visuals/dom/image_data.h"

#include "base/strings/utf_string_conversions.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ImageData
//
ImageData::ImageData(base::StringPiece16 value) : value_(value.as_string()) {}
ImageData::ImageData(const ImageData& other) : ImageData(other.value_) {}
ImageData::~ImageData() {}

ImageData& ImageData::operator=(const ImageData& other) {
  value_ = other.value_;
  return *this;
}

bool ImageData::operator==(const ImageData& other) const {
  if (this == &other)
    return true;
  return value_ == other.value_;
}

bool ImageData::operator!=(const ImageData& other) const {
  return !operator==(other);
}

std::ostream& operator<<(std::ostream& ostream, const ImageData& data) {
  return ostream << "ImageData(" << base::UTF16ToUTF8(data.value()) << ')';
}

}  // namespace visuals
