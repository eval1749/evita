// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_PAINT_PUBLIC_LINE_INLINE_BOX_FORWARD_H_
#define EVITA_PAINT_PUBLIC_LINE_INLINE_BOX_FORWARD_H_

namespace paint {

#define FOR_EACH_PAINT_INLINE_BOX(V) \
  V(InlineFillerBox)                 \
  V(InlineMarkerBox)                 \
  V(InlineTextBox)                   \
  V(InlineUnicodeBox)

// Forward declarations
class InlineBox;
#define V(name) class name;
FOR_EACH_PAINT_INLINE_BOX(V)
#undef V
class InlineBoxVisitor;

}  // namespace paint

#endif  // EVITA_PAINT_PUBLIC_LINE_INLINE_BOX_FORWARD_H_
