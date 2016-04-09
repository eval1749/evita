// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_IMAGE_H_
#define EVITA_VISUALS_DOM_IMAGE_H_

#include "evita/visuals/dom/node.h"

#include "evita/visuals/dom/image_data.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Image
//
class Image final : public Node {
  DECLARE_VISUAL_NODE_FINAL_CLASS(Image, Node);

 public:
  Image(Document* document, const ImageData& data);
  ~Image() final;

  const ImageData& data() const { return data_; }
  float opacity() const { return opacity_; }

 private:
  ImageData data_;
  float opacity_ = 1.0f;

  DISALLOW_COPY_AND_ASSIGN(Image);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_IMAGE_H_
