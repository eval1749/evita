// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/checkbox_painter.h"

#include "evita/gfx/brush.h"
#include "evita/gfx/canvas.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// CheckboxPainter
//
CheckboxPainter::CheckboxPainter() {}
CheckboxPainter::~CheckboxPainter() {}

void CheckboxPainter::Paint(gfx::Canvas* canvas,
                            const gfx::RectF& bounds,
                            Control::State state,
                            const Control::Style& style,
                            bool checked) const {
  DCHECK(!bounds.empty());

  canvas->FillRectangle(gfx::Brush(canvas, style.bgcolor), bounds);

  auto const frame_size = gfx::SizeF(12.0f, 12.0f);
  auto const offset = (bounds.size() - frame_size) / 2;
  gfx::RectF frame_rect(bounds.origin() + offset, frame_size);
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, frame_rect);

  gfx::Brush frame_brush(canvas, style.shadow);
  (*canvas)->DrawRectangle(frame_rect, frame_brush);

  if (checked) {
    gfx::Brush black_brush(canvas, gfx::ColorF(0, 0, 0));
    (*canvas)->DrawLine(gfx::PointF(frame_rect.left + 3, frame_rect.top + 6),
                        gfx::PointF(frame_rect.left + 6, frame_rect.bottom - 3),
                        black_brush, 2.0f);
    (*canvas)->DrawLine(gfx::PointF(frame_rect.left + 6, frame_rect.top + 9),
                        gfx::PointF(frame_rect.right - 3, frame_rect.top + 3),
                        black_brush, 2.0f);
  }
  switch (state) {
    case Control::State::Disabled:
    case Control::State::Normal:
      break;
    case Control::State::Highlight:
      canvas->FillRectangle(
          gfx::Brush(canvas, gfx::ColorF(style.highlight, 0.1f)), frame_rect);
      canvas->DrawRectangle(
          gfx::Brush(canvas, gfx::ColorF(style.highlight, 0.1f)), frame_rect);
      break;
    case Control::State::Hovered:
      canvas->FillRectangle(
          gfx::Brush(canvas, gfx::ColorF(style.hotlight, 0.1f)), frame_rect);
      canvas->DrawRectangle(
          gfx::Brush(canvas, gfx::ColorF(style.hotlight, 0.1f)), frame_rect);
      break;
  }
  canvas->AddDirtyRect(bounds);
}

}  // namespace ui
