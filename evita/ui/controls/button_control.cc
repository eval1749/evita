// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/button_control.h"

#include "evita/gfx/canvas.h"
#include "evita/gfx/text_format.h"
#include "evita/gfx/text_layout.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// ButtonControl::Renderer
//
class ButtonControl::Renderer {
  private: gfx::RectF bounds_;
  private: Style style_;
  private: std::unique_ptr<gfx::TextLayout> text_layout_;
  private: gfx::SizeF text_size_;

  public: Renderer(const base::string16& text, const Style& style,
                   const gfx::RectF& rect);
  public: ~Renderer();

  public: void Render(gfx::Canvas* canvas, Control::State state) const;

  DISALLOW_COPY_AND_ASSIGN(Renderer);
};

namespace {
std::unique_ptr<gfx::TextLayout> CreateTextLayout(const base::string16& text,
    const ButtonControl::Style& style, const gfx::SizeF& size) {
  gfx::TextFormat text_format(style.font_family, style.font_size);
  common::ComPtr<IDWriteInlineObject> inline_object;
  COM_VERIFY(gfx::FactorySet::instance()->dwrite().
      CreateEllipsisTrimmingSign(text_format, &inline_object));
  DWRITE_TRIMMING trimming {DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0};
  text_format->SetTrimming(&trimming, inline_object);
  return text_format.CreateLayout(text, size);
}
}  // namespace

ButtonControl::Renderer::Renderer(const base::string16& text,
                                  const Style& style,
                                  const gfx::RectF& rect)
    : bounds_(rect), style_(style),
      text_layout_(CreateTextLayout(text, style, rect.size())) {
  DWRITE_TEXT_METRICS metrics;
  COM_VERIFY((*text_layout_)->GetMetrics(&metrics));
  // Event after trimming of |text_|, it may not fit into line. Although,
  // DCHECK_EQ(1u, metrics.lineCount);
  text_size_ = gfx::SizeF(metrics.width, metrics.height);
}

ButtonControl::Renderer::~Renderer() {
}

void ButtonControl::Renderer::Render(gfx::Canvas* canvas,
                                     Control::State state) const {
  if (!bounds_)
    return;

  canvas->FillRectangle(gfx::Brush(canvas, style_.bgcolor), bounds_);

  auto frame_rect = bounds_;
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, frame_rect);
  canvas->DrawRectangle(gfx::Brush(canvas, style_.shadow), frame_rect);

  auto const offset = (frame_rect.size() - text_size_) / 2.0f;
  gfx::PointF origin(frame_rect.origin() + offset);
  gfx::Brush text_brush(canvas, state == State::Disabled ? style_.gray_text :
                                                           style_.color);
  (*canvas)->DrawTextLayout(origin, *text_layout_, text_brush,
                         D2D1_DRAW_TEXT_OPTIONS_CLIP);

  switch (state) {
    case Control::State::Disabled:
    case Control::State::Normal:
      break;
    case Control::State::Highlight:
      canvas->FillRectangle(
          gfx::Brush(canvas, gfx::ColorF(style_.highlight, 0.1f)),
          frame_rect);
      canvas->DrawRectangle(gfx::Brush(canvas, style_.highlight), frame_rect);
      break;
    case Control::State::Hovered:
      canvas->FillRectangle(
          gfx::Brush(canvas, gfx::ColorF(style_.hotlight, 0.1f)),
          frame_rect);
      canvas->DrawRectangle(gfx::Brush(canvas, style_.hotlight), frame_rect);
      break;
  }
  canvas->AddDirtyRect(bounds_);
}

//////////////////////////////////////////////////////////////////////
//
// ButtonControl
//
ButtonControl::ButtonControl(ControlController* controller,
                           const base::string16& text, const Style& style)
    : Control(controller), style_(style), text_(text) {
}

ButtonControl::~ButtonControl() {
}

void ButtonControl::set_style(const Style& new_style) {
  if (style_ == new_style)
    return;
  style_ = new_style;
  renderer_.reset();
  SchedulePaint();
}

void ButtonControl::set_text(const base::string16& new_text) {
  if (text_ == new_text)
    return;
  text_ = new_text;
  renderer_.reset();
  SchedulePaint();
}

// ui::Widget
void ButtonControl::DidChangeBounds() {
  Control::DidChangeBounds();
  renderer_.reset();
}

void ButtonControl::OnDraw(gfx::Canvas* canvas) {
  if (!renderer_)
    renderer_ = std::make_unique<Renderer>(text_, style_, GetContentsBounds());
  renderer_->Render(canvas, state());
}

}  // namespace ui
