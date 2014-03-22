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
CheckboxControl::CheckboxControl(bool checked, const Style& style)
    : checked_(checked), style_(style) {
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
  auto const width = 13.0f;
  auto const height = 13.0f;
  gfx::RectF box_rect(
      gfx::PointF(rect().left + (rect().width() - width) / 2,
                  rect().top + (rect().height() - height) / 2),
      gfx::SizeF(width, height));
  gfx->FillRectangle(gfx::Brush(*gfx, style_.bgcolor), rect());
  gfx::Brush gray_brush(*gfx, gfx::ColorF::DarkGray);
  auto const alias_mode = (*gfx)->GetAntialiasMode();
  (*gfx)->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
  gfx->DrawRectangle(gray_brush, box_rect);
  (*gfx)->SetAntialiasMode(alias_mode);
  if (checked_) {
    gfx::Brush black_brush(*gfx, gfx::ColorF(0, 0, 0));
    (*gfx)->DrawLine(gfx::PointF(box_rect.left + 3, box_rect.top + 6),
                     gfx::PointF(box_rect.left + 6, box_rect.bottom - 3),
                     black_brush,
                     2.0f);
    (*gfx)->DrawLine(gfx::PointF(box_rect.left + 6, box_rect.top + 9),
                     gfx::PointF(box_rect.right - 3, box_rect.top + 3),
                     black_brush,
                     2.0f);
  }
  gfx->Flush();
}

}  // namespace ui
