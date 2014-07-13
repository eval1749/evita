// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/checkbox_control.h"

#include "evita/gfx_base.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// CheckboxControl
//
CheckboxControl::CheckboxControl(ControlController* controller,
                                 bool checked, const Style& style)
    : Control(controller), checked_(checked), style_(style) {
}

CheckboxControl::~CheckboxControl() {
}

void CheckboxControl::set_style(const Style& new_style) {
  style_ = new_style;
}

void CheckboxControl::set_checked(bool new_checked) {
  checked_ = new_checked;
}

// ui::Widget
void CheckboxControl::OnDraw(gfx::Canvas* canvas) {
  if (!bounds())
    return;

  auto const rect = gfx::RectF(this->bounds());
  canvas->FillRectangle(gfx::Brush(canvas, style_.bgcolor), rect);

  auto const frame_size = gfx::SizeF(12.0f, 12.0f);
  auto const offset = (rect.size() - frame_size) / 2;
  gfx::RectF frame_rect(rect.left_top() + offset, frame_size);
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, frame_rect);

  gfx::Brush frame_brush(canvas, style_.shadow);
  (*canvas)->DrawRectangle(frame_rect, frame_brush);

  if (checked_) {
    gfx::Brush black_brush(canvas, gfx::ColorF(0, 0, 0));
    (*canvas)->DrawLine(gfx::PointF(frame_rect.left + 3, frame_rect.top + 6),
                     gfx::PointF(frame_rect.left + 6, frame_rect.bottom - 3),
                     black_brush,
                     2.0f);
    (*canvas)->DrawLine(gfx::PointF(frame_rect.left + 6, frame_rect.top + 9),
                     gfx::PointF(frame_rect.right - 3, frame_rect.top + 3),
                     black_brush,
                     2.0f);
  }
  switch (state()) {
    case Control::State::Disabled:
    case Control::State::Normal:
      break;
    case Control::State::Highlight:
      canvas->FillRectangle(
          gfx::Brush(canvas, gfx::ColorF(style_.highlight, 0.1f)),
          frame_rect);
      canvas->DrawRectangle(
        gfx::Brush(canvas, gfx::ColorF(style_.highlight, 0.1f)),
        frame_rect);
      break;
    case Control::State::Hover:
      canvas->FillRectangle(
          gfx::Brush(canvas, gfx::ColorF(style_.hotlight, 0.1f)),
          frame_rect);
      canvas->DrawRectangle(
          gfx::Brush(canvas, gfx::ColorF(style_.hotlight, 0.1f)),
          frame_rect);
      break;
  }
  canvas->Flush();
}

}  // namespace ui
