// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/paint/layout_view_painter.h"

#include "evita/gfx_base.h"
#include "evita/views/text/layout_view.h"

namespace paint {

LayoutViewPainter::LayoutViewPainter(gfx::Canvas* canvas) : canvas_(canvas) {}

LayoutViewPainter::~LayoutViewPainter() {}

void LayoutViewPainter::Paint(const views::LayoutView& layout_view) {
  const auto& ruler_bounds = layout_view.ruler_bounds();
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas_, ruler_bounds);
  // TODO(eval1749): We should get ruler color from CSS.
  gfx::Brush brush(canvas_, gfx::ColorF(0, 0, 0, 0.3f));
  canvas_->DrawRectangle(brush, ruler_bounds);
}

}  // namespace pain
