// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/layout/box_visitor.h"

#include "evita/visuals/layout/flow_box.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/layout/shape_box.h"
#include "evita/visuals/layout/text_box.h"

namespace visuals {

BoxVisitor::BoxVisitor() {}
BoxVisitor::~BoxVisitor() {}

void BoxVisitor::Visit(const Box& box) {
  Visit(&const_cast<Box&>(box));
}

void BoxVisitor::Visit(Box* box) {
  box->Accept(this);
}

#define V(name) \
  void name::Accept(BoxVisitor* visitor) { visitor->Visit##name(this); }
FOR_EACH_VISUAL_BOX(V)
#undef V

}  // namespace visuals
