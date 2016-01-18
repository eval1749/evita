// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_DOM_SHAPE_H_
#define EVITA_VISUALS_DOM_SHAPE_H_

#include "evita/visuals/dom/node.h"

#include "evita/visuals/dom/shape_data.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Shape
//
class Shape final : public Node {
  DECLARE_VISUAL_NODE_FINAL_CLASS(Shape, Node);

 public:
  Shape(Document* document, const ShapeData& data);
  ~Shape() final;

  const ShapeData& data() const { return data_; }

 private:
  ShapeData data_;

  DISALLOW_COPY_AND_ASSIGN(Shape);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_DOM_SHAPE_H_
