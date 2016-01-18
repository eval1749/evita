// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_SHAPE_BOX_H_
#define EVITA_VISUALS_LAYOUT_SHAPE_BOX_H_

#include <vector>

#include "evita/visuals/layout/content_box.h"

#include "evita/visuals/css/values.h"
#include "evita/visuals/dom/shape_data.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ShapeBox
//
class ShapeBox final : public ContentBox {
  DECLARE_VISUAL_BOX_FINAL_CLASS(ShapeBox, ContentBox);

 public:
  ShapeBox(RootBox* root_box, const ShapeData& data, const Node* node);
  ~ShapeBox() final;

  const FloatColor& color() const { return color_; }
  const ShapeData& data() const { return data_; }
  const css::FontSize& size() const { return font_size_; }

 private:
  FloatColor color_;
  ShapeData data_;
  css::FontSize font_size_;

  DISALLOW_COPY_AND_ASSIGN(ShapeBox);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_SHAPE_BOX_H_
