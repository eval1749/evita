// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_PAINT_INLINE_BOX_PAINTER_H_
#define EVITA_PAINT_INLINE_BOX_PAINTER_H_

#include "base/macros.h"
#include "evita/gfx/forward.h"
#include "evita/views/text/inline_box_forward.h"
#include "evita/views/text/inline_box_visitor.h"

namespace paint {

//////////////////////////////////////////////////////////////////////
//
// InlineBoxPainter
//
class InlineBoxPainter final : public views::rendering::InlineBoxVisitor {
 public:
  InlineBoxPainter(gfx::Canvas* canvas, const gfx::RectF& rect);
  ~InlineBoxPainter();

  void Paint(const views::rendering::InlineBox& inline_box);

 private:
#define V(name) void Visit##name(views::rendering::name* inline_box) final;
  FOR_EACH_INLINE_BOX(V)
#undef V

  gfx::Canvas* const canvas_;
  const gfx::RectF& rect_;

  DISALLOW_COPY_AND_ASSIGN(InlineBoxPainter);
};

}  // namespace paint

#endif  // EVITA_PAINT_INLINE_BOX_PAINTER_H_
