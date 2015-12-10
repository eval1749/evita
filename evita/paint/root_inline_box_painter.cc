// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/paint/root_inline_box_painter.h"

#include "evita/gfx_base.h"
#include "evita/paint/inline_box_painter.h"
#include "evita/views/text/inline_box.h"
#include "evita/views/text/root_inline_box.h"

namespace paint {

using RootInlineBox = views::rendering::RootInlineBox;

//////////////////////////////////////////////////////////////////////
//
// RootInlineBoxPainter
//
RootInlineBoxPainter::RootInlineBoxPainter(gfx::Canvas* canvas)
    : canvas_(canvas) {}

RootInlineBoxPainter::~RootInlineBoxPainter() {}

void RootInlineBoxPainter::Paint(const RootInlineBox& root_box) {
  auto const& bounds = root_box.bounds();
  auto x = bounds.left;
  for (auto inline_box : root_box.cells()) {
    gfx::RectF rect(x, bounds.top, x + inline_box->width(),
                    ::ceil(bounds.top + inline_box->line_height()));
    paint::InlineBoxPainter painter(canvas_, rect);
    painter.Paint(*inline_box);
    x = rect.right;
  }
}

}  // namespace paint
