// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_IMAGING_IMAGE_BITMAP_H_
#define EVITA_VISUALS_IMAGING_IMAGE_BITMAP_H_

#include <stdint.h>

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "evita/gfx/base/geometry/float_size.h"

namespace visuals {

class NativeImageBitmap;

//////////////////////////////////////////////////////////////////////
//
// ImageBitmap
//
class ImageBitmap final {
 public:
  ImageBitmap(const void* data, size_t data_size, const gfx::FloatSize& size);
  explicit ImageBitmap(std::unique_ptr<NativeImageBitmap> native_image);
  explicit ImageBitmap(const gfx::FloatSize& size);
  ImageBitmap(const ImageBitmap& other);
  ImageBitmap(ImageBitmap&& other);
  ImageBitmap();
  ~ImageBitmap();

  ImageBitmap& operator=(const ImageBitmap& other);
  ImageBitmap& operator=(ImageBitmap&& other);

  bool operator==(const ImageBitmap& other) const;
  bool operator!=(const ImageBitmap& other) const;

  std::vector<uint8_t> data() const;
  base::string16 format() const;
  const NativeImageBitmap& impl() const { return *impl_; }
  bool is_valid() const { return static_cast<bool>(impl_); }
  gfx::FloatSize resolution() const;
  const gfx::FloatSize& size() const;

  static ImageBitmap Decode(base::StringPiece16 format,
                            const void* data,
                            size_t data_size);

  std::vector<uint8_t> Encode(base::StringPiece16 format) const;

 private:
  std::unique_ptr<NativeImageBitmap> impl_;
};

std::ostream& operator<<(std::ostream& ostream, const ImageBitmap& image);

}  // namespace visuals

#endif  // EVITA_VISUALS_IMAGING_IMAGE_BITMAP_H_
