// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_CSS_PROPERTY_VISITOR_H_
#define EVITA_VISUALS_CSS_PROPERTY_VISITOR_H_

#include "base/macros.h"
#include "evita/visuals/css/properties_forward.h"

namespace visuals {
namespace css {

class Style;

//////////////////////////////////////////////////////////////////////
//
// PropertyVisitor
//
class PropertyVisitor {
 public:
  void Visit(const Style& style);

#define V(Name, name, type, text) \
  virtual void Visit##Name(const Name##Property& property) = 0;  // NOLINT
  FOR_EACH_VISUAL_CSS_PROPERTY(V)
#undef V
 protected:
  PropertyVisitor();
  virtual ~PropertyVisitor();

 private:
  DISALLOW_COPY_AND_ASSIGN(PropertyVisitor);
};

}  // namespace css
}  // namespace visuals

#endif  // EVITA_VISUALS_CSS_PROPERTY_VISITOR_H_
