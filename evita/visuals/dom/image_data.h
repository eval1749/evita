// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_IMAGE_DATA_H_
#define EVITA_VISUALS_DOM_IMAGE_DATA_H_

#include <stdint.h>

#include <iosfwd>

#include "base/strings/string_piece.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ImageData
//
class ImageData final {
 public:
  explicit ImageData(base::StringPiece16 value);
  ImageData(const ImageData& other);
  ~ImageData();

  ImageData& operator=(const ImageData& other);

  bool operator==(const ImageData& other) const;
  bool operator!=(const ImageData& other) const;

  const base::string16& value() const { return value_; }

 private:
  base::string16 value_;
};

std::ostream& operator<<(std::ostream& ostream, const ImageData& data);

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_IMAGE_DATA_H_
