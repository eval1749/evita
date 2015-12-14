// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_INLINE_BOX_VISITOR_H_
#define EVITA_LAYOUT_INLINE_BOX_VISITOR_H_

#include "base/macros.h"
#include "evita/layout/inline_box_forward.h"

namespace layout {

//////////////////////////////////////////////////////////////////////
//
// InlineBoxVisitor
//
class InlineBoxVisitor {
 public:
#define V(name) virtual void Visit##name(name* inline_box) = 0;
  FOR_EACH_INLINE_BOX(V)
#undef V
 protected:
  InlineBoxVisitor();
  virtual ~InlineBoxVisitor();

 private:
  DISALLOW_COPY_AND_ASSIGN(InlineBoxVisitor);
};

}  // namespace layout

#endif  // EVITA_LAYOUT_INLINE_BOX_VISITOR_H_
