// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/shape.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// Shape
//
Shape::Shape(Document* document, const ShapeData& data)
    : Node(document, L"#shape"), data_(data) {}

Shape::~Shape() {}

}  // namespace visuals
