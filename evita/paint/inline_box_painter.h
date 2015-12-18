// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_PAINT_INLINE_BOX_PAINTER_H_
#define EVITA_PAINT_INLINE_BOX_PAINTER_H_

#include "base/macros.h"
#include "evita/gfx/forward.h"
#include "evita/paint/public/line/inline_box_forward.h"

namespace paint {

//////////////////////////////////////////////////////////////////////
//
// InlineBoxPainter
//
class InlineBoxPainter final {
 public:
  explicit InlineBoxPainter(const InlineBox& inline_box);
  ~InlineBoxPainter();

  void Paint(gfx::Canvas* canvas, const gfx::RectF& rect);

 private:
  const InlineBox& inline_box_;

  DISALLOW_COPY_AND_ASSIGN(InlineBoxPainter);
};

}  // namespace paint

#endif  // EVITA_PAINT_INLINE_BOX_PAINTER_H_
