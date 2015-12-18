// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_PAINT_PUBLIC_LINE_INLINE_BOX_VISITOR_H_
#define EVITA_PAINT_PUBLIC_LINE_INLINE_BOX_VISITOR_H_

#include "base/macros.h"
#include "evita/paint/public/line/inline_box_forward.h"

namespace paint {

//////////////////////////////////////////////////////////////////////
//
// InlineBoxVisitor
//
class InlineBoxVisitor {
 public:
#define V(name) virtual void Visit##name(name* inline_box) = 0;
  FOR_EACH_PAINT_INLINE_BOX(V)
#undef V
 protected:
  InlineBoxVisitor();
  virtual ~InlineBoxVisitor();

 private:
  DISALLOW_COPY_AND_ASSIGN(InlineBoxVisitor);
};

}  // namespace paint

#endif  // EVITA_PAINT_PUBLIC_LINE_INLINE_BOX_VISITOR_H_
