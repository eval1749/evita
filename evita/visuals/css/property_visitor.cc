// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/property_visitor.h"
#include "evita/visuals/css/style.h"

namespace visuals {
namespace css {

//////////////////////////////////////////////////////////////////////
//
// PropertyVisitor
//
PropertyVisitor::PropertyVisitor() {}
PropertyVisitor::~PropertyVisitor() {}

void PropertyVisitor::Visit(const Style& style) {
#define V(Name, name, type, text) \
  if (style.has_##name())         \
    Visit##Name(Name##Property(style.name()));

  FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V
}

#define V(Name, name, type, text)                         \
  void Name##Property::Accept(PropertyVisitor* visitor) { \
    visitor->Visit##Name(*this);                          \
  }

FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V

}  // namespace css
}  // namespace visuals
