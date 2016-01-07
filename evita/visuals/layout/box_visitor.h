// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_LAYOUT_BOX_VISITOR_H_
#define EVITA_VISUALS_LAYOUT_BOX_VISITOR_H_

#include "base/macros.h"
#include "evita/visuals/layout/box_forward.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// BoxVisitor
//
class BoxVisitor {
 public:
  void Visit(const Box& box);
  void Visit(Box* box);

#define V(name) virtual void Visit##name(name* box) = 0;
  FOR_EACH_VISUAL_BOX(V)
#undef V
 protected:
  BoxVisitor();
  virtual ~BoxVisitor();

 private:
  DISALLOW_COPY_AND_ASSIGN(BoxVisitor);
};

}  // namespace visuals

#endif  // EVITA_VISUALS_LAYOUT_BOX_VISITOR_H_
