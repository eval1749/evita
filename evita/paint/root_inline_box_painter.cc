// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/paint/root_inline_box_painter.h"

#include "evita/gfx_base.h"
#include "evita/layout/line/inline_box.h"
#include "evita/paint/inline_box_painter.h"
#include "evita/paint/public/line/root_inline_box.h"

namespace paint {

//////////////////////////////////////////////////////////////////////
//
// RootInlineBoxPainter
//
RootInlineBoxPainter::RootInlineBoxPainter(const RootInlineBox& root_box)
    : root_box_(root_box) {}

RootInlineBoxPainter::~RootInlineBoxPainter() {}

void RootInlineBoxPainter::Paint(gfx::Canvas* canvas) {
  auto const& bounds = root_box_.bounds();
  auto x = bounds.left;
  for (auto box : root_box_.boxes()) {
    // TODO(eval1749): We should not use |ceil()| here.
    gfx::RectF rect(x, bounds.top, x + box->width(),
                    ::ceil(bounds.top + box->line_height()));
    InlineBoxPainter(*box).Paint(canvas, rect);
    x = rect.right;
  }
}

}  // namespace paint
