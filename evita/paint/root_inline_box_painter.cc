// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/paint/root_inline_box_painter.h"

#include "evita/gfx_base.h"
#include "evita/paint/inline_box_painter.h"
#include "evita/layout/inline_box.h"
#include "evita/layout/root_inline_box.h"

namespace paint {

using RootInlineBox = views::rendering::RootInlineBox;

//////////////////////////////////////////////////////////////////////
//
// RootInlineBoxPainter
//
RootInlineBoxPainter::RootInlineBoxPainter(const RootInlineBox& root_inline_box)
    : root_inline_box_(root_inline_box) {}

RootInlineBoxPainter::~RootInlineBoxPainter() {}

void RootInlineBoxPainter::Paint(gfx::Canvas* canvas) {
  auto const& bounds = root_inline_box_.bounds();
  auto x = bounds.left;
  for (auto inline_box : root_inline_box_.cells()) {
    gfx::RectF rect(x, bounds.top, x + inline_box->width(),
                    ::ceil(bounds.top + inline_box->line_height()));
    InlineBoxPainter(*inline_box).Paint(canvas, rect);
    x = rect.right;
  }
}

}  // namespace paint
