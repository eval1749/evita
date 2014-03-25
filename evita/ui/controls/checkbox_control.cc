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
void CheckboxControl::OnDraw(gfx::Graphics* gfx) {
  if (rect().empty())
    return;
  auto const size = 12.0f;
  gfx->FillRectangle(gfx::Brush(*gfx, style_.bgcolor), rect());

  gfx::RectF frame_rect(
      gfx::PointF(rect().left + (rect().width() - size) / 2,
                  rect().top + (rect().height() - size) / 2),
      gfx::SizeF(size, size));

  if (hover()) {
    gfx::Brush frame_brush(*gfx, style_.highlight);
    (*gfx)->DrawRectangle(frame_rect, frame_brush);
  } else {
    auto const antialias_mode = (*gfx)->GetAntialiasMode();
    //(*gfx)->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    gfx::Brush frame_brush(*gfx, style_.shadow);
    (*gfx)->DrawRectangle(frame_rect, frame_brush);
    (*gfx)->SetAntialiasMode(antialias_mode);
  }

  if (checked_) {
    gfx::Brush black_brush(*gfx, gfx::ColorF(0, 0, 0));
    (*gfx)->DrawLine(gfx::PointF(frame_rect.left + 3, frame_rect.top + 6),
                     gfx::PointF(frame_rect.left + 6, frame_rect.bottom - 3),
                     black_brush,
                     2.0f);
    (*gfx)->DrawLine(gfx::PointF(frame_rect.left + 6, frame_rect.top + 9),
                     gfx::PointF(frame_rect.right - 3, frame_rect.top + 3),
                     black_brush,
                     2.0f);
  }
  gfx->Flush();
}

}  // namespace ui
