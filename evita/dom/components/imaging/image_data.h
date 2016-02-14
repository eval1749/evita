// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_COMPONENTS_IMAGING_IMAGE_DATA_H_
#define EVITA_DOM_COMPONENTS_IMAGING_IMAGE_DATA_H_

#include <string>
#include <vector>

#include "evita/ginx/scriptable.h"

#include "base/macros.h"
#include "base/strings/string16.h"
#include "evita/visuals/imaging/image_bitmap.h"

namespace gin {
class ArrayBufferView;
}

namespace dom {

class ExceptionState;

namespace bindings {
class ImageDataClass;
}

//////////////////////////////////////////////////////////////////////
//
// ImageData
//
class ImageData final : public ginx::Scriptable<ImageData> {
  DECLARE_SCRIPTABLE_OBJECT(ImageData);

 public:
  explicit ImageData(const visuals::ImageBitmap& bitmap);
  ~ImageData() final;

 private:
  friend class bindings::ImageDataClass;

  // bindings
  ImageData(int width, int height, ExceptionState* exception_state);
  ImageData(const gin::ArrayBufferView& input,
            int width,
            int height,
            ExceptionState* exception_state);

  // Returns 32-bit RGBA array.
  std::vector<uint8_t> data() const;
  base::string16 format() const;
  int height() const;
  float resolution_x() const;
  float resolution_y() const;
  int width() const;

  static ImageData* Decode(const base::string16& mime_type,
                           const gin::ArrayBufferView& input,
                           ExceptionState* exception_state);

  std::vector<uint8_t> Encode(const base::string16& mime_type,
                              ExceptionState* exception_state) const;

  const visuals::ImageBitmap bitmap_;

  DISALLOW_COPY_AND_ASSIGN(ImageData);
};

}  // namespace dom

#endif  // EVITA_DOM_COMPONENTS_IMAGING_IMAGE_DATA_H_
