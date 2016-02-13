// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_IMAGE_BOX_H_
#define EVITA_VISUALS_LAYOUT_IMAGE_BOX_H_

#include <vector>

#include "evita/visuals/layout/content_box.h"

#include "evita/visuals/css/values.h"
#include "evita/visuals/dom/image_data.h"

namespace visuals {

class ImageBitmap;

//////////////////////////////////////////////////////////////////////
//
// ImageBox
//
class ImageBox final : public ContentBox {
  DECLARE_VISUAL_BOX_FINAL_CLASS(ImageBox, ContentBox);

 public:
  ImageBox(RootBox* root_box, const ImageData& data, const Node* node);
  ~ImageBox() final;

  const ImageData& data() const { return data_; }
  const ImageBitmap& bitmap() const { return *bitmap_; }

 private:
  ImageData data_;
  std::unique_ptr<ImageBitmap> bitmap_;

  DISALLOW_COPY_AND_ASSIGN(ImageBox);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_IMAGE_BOX_H_
