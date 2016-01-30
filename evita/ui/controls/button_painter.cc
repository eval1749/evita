// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/button_painter.h"

#include "evita/gfx/brush.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/direct_write_factory_win.h"
#include "evita/gfx/text_format.h"
#include "evita/gfx/text_layout.h"

namespace ui {

namespace {
std::unique_ptr<gfx::TextLayout> CreateTextLayout(base::StringPiece16 text,
                                                  const Control::Style& style,
                                                  const gfx::SizeF& size) {
  gfx::TextFormat text_format(style.font_family, style.font_size);
  common::ComPtr<IDWriteInlineObject> inline_object;
  COM_VERIFY(gfx::DirectWriteFactory::GetInstance()
                 ->impl()
                 ->CreateEllipsisTrimmingSign(text_format, &inline_object));
  DWRITE_TRIMMING trimming{DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
  text_format->SetTrimming(&trimming, inline_object);
  // TODO(eval1749): We should have |base::StringPiece16| version of
  // |gfx::TextFormat::CreateLayout()|.
  return text_format.CreateLayout(text.as_string(), size);
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ButtonPainter
//
ButtonPainter::ButtonPainter() {}
ButtonPainter::~ButtonPainter() {}

void ButtonPainter::Paint(gfx::Canvas* canvas,
                          const gfx::RectF& bounds,
                          Control::State state,
                          const Control::Style& style,
                          base::StringPiece16 text) const {
  const auto& text_layout = CreateTextLayout(text, style, bounds.size());
  DWRITE_TEXT_METRICS metrics;
  COM_VERIFY((*text_layout)->GetMetrics(&metrics));
  // Event after trimming of |text_|, it may not fit into line. Although,
  // DCHECK_EQ(1u, metrics.lineCount);
  const auto& text_size = gfx::SizeF(metrics.width, metrics.height);

  canvas->FillRectangle(gfx::Brush(canvas, style.bgcolor), bounds);

  auto frame_rect = bounds;
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, frame_rect);
  canvas->DrawRectangle(gfx::Brush(canvas, style.shadow), frame_rect);

  auto const offset = (frame_rect.size() - text_size) / 2.0f;
  gfx::PointF origin(frame_rect.origin() + offset);
  gfx::Brush text_brush(canvas, state == Control::State::Disabled
                                    ? style.gray_text
                                    : style.color);
  (*canvas)->DrawTextLayout(origin, *text_layout, text_brush,
                            D2D1_DRAW_TEXT_OPTIONS_CLIP);

  switch (state) {
    case Control::State::Disabled:
    case Control::State::Normal:
      break;
    case Control::State::Highlight:
      canvas->FillRectangle(
          gfx::Brush(canvas, gfx::ColorF(style.highlight, 0.1f)), frame_rect);
      canvas->DrawRectangle(gfx::Brush(canvas, style.highlight), frame_rect);
      break;
    case Control::State::Hovered:
      canvas->FillRectangle(
          gfx::Brush(canvas, gfx::ColorF(style.hotlight, 0.1f)), frame_rect);
      canvas->DrawRectangle(gfx::Brush(canvas, style.hotlight), frame_rect);
      break;
  }
  canvas->AddDirtyRect(bounds);
}

}  // namespace ui
