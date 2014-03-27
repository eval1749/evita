// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/text_field_control.h"

#include "evita/gfx_base.h"

namespace ui {

namespace {
auto const margin_size = 2.0f;
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TextFieldControl::Renderer
//
class TextFieldControl::Renderer {
  private: gfx::RectF rect_;
  private: Style style_;
  private: std::unique_ptr<gfx::TextLayout> text_layout_;
  private: gfx::SizeF text_size_;

  public: Renderer(const base::string16& text, const Style& style,
                   const gfx::RectF& rect);
  public: ~Renderer();

  public: void Render(gfx::Graphics* gfx, Control::State state) const;

  DISALLOW_COPY_AND_ASSIGN(Renderer);
};

namespace {
std::unique_ptr<gfx::TextLayout> CreateTextLayout(const base::string16& text,
    const TextFieldControl::Style& style, const gfx::SizeF& size) {
  gfx::TextFormat text_format(style.font_family, style.font_size);
  return text_format.CreateLayout(text, size);
}
}  // namespace

TextFieldControl::Renderer::Renderer(const base::string16& text,
                                  const Style& style,
                                  const gfx::RectF& rect)
    : rect_(rect), style_(style),
      text_layout_(CreateTextLayout(text, style, rect.size() - margin_size)) {
  DWRITE_TEXT_METRICS metrics;
  COM_VERIFY((*text_layout_)->GetMetrics(&metrics));
  // Event after trimming of |text_|, it may not fit into line. Although,
  // DCHECK_EQ(1u, metrics.lineCount);
  text_size_ = gfx::SizeF(metrics.width, metrics.height);
}

TextFieldControl::Renderer::~Renderer() {
}

void TextFieldControl::Renderer::Render(gfx::Graphics* gfx,
                                     Control::State state) const {
  if (!rect_)
    return;

  gfx->FillRectangle(gfx::Brush(*gfx, style_.bgcolor), rect_);

  const auto frame_rect = rect_;
  gfx::Graphics::AxisAlignedClipScope clip_scope(*gfx, frame_rect);
  gfx->DrawRectangle(gfx::Brush(*gfx, style_.shadow), frame_rect);

  {
    auto const offset = (frame_rect.size() - text_size_) / 2.0f;
    gfx::PointF text_origin(frame_rect.left + margin_size,
                            frame_rect.top + offset.height);
    gfx::Brush text_brush(*gfx, state == State::Disabled ? style_.gray_text :
                                                           style_.color);
    const auto text_rect = rect_ - margin_size;
    gfx::Graphics::AxisAlignedClipScope clip_scope(*gfx, text_rect);
    (*gfx)->DrawTextLayout(text_origin, *text_layout_, text_brush,
                           D2D1_DRAW_TEXT_OPTIONS_CLIP);
  }

  switch (state) {
    case Control::State::Disabled:
      gfx->FillRectangle(
          gfx::Brush(*gfx, gfx::ColorF(style_.gray_text, 0.5f)),
          frame_rect);
      gfx->DrawRectangle(gfx::Brush(*gfx, style_.color), frame_rect);
      break;
    case Control::State::Normal:
      break;
    case Control::State::Highlight:
      gfx->DrawRectangle(gfx::Brush(*gfx, gfx::ColorF(style_.highlight, 0.5f)),
                         frame_rect - 1.0f, 2.0f);
      break;
    case Control::State::Hover:
      gfx->DrawRectangle(gfx::Brush(*gfx, style_.hotlight), frame_rect);
      break;
  }
  gfx->Flush();
}

//////////////////////////////////////////////////////////////////////
//
// TextFieldControl
//
TextFieldControl::TextFieldControl(ControlController* controller,
                           const base::string16& text, const Style& style)
    : Control(controller), style_(style), text_(text) {
}

TextFieldControl::~TextFieldControl() {
}

void TextFieldControl::set_style(const Style& new_style) {
  if (style_ == new_style)
    return;
  style_ = new_style;
  renderer_.reset();
}

void TextFieldControl::set_text(const base::string16& new_text) {
  if (text_ == new_text)
    return;
  text_ = new_text;
  renderer_.reset();
}

// ui::Widget
void TextFieldControl::DidResize() {
  renderer_.reset();
}

void TextFieldControl::OnDraw(gfx::Graphics* gfx) {
  if (!renderer_)
    renderer_ = std::make_unique<Renderer>(text_, style_, gfx::RectF(rect()));
  renderer_->Render(gfx, state());
}

}  // namespace ui
