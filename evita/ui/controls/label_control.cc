// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/label_control.h"

#include "evita/gfx/canvas.h"
#include "evita/gfx/text_format.h"
#include "evita/gfx/text_layout.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// LabelControl::Renderer
//
class LabelControl::Renderer {
  private: gfx::RectF bounds_;
  private: gfx::PointF text_origin_;
  private: Style style_;
  private: std::unique_ptr<gfx::TextLayout> text_layout_;

  public: Renderer(const base::string16& text, const Style& style,
                   const gfx::RectF& rect);
  public: ~Renderer();

  public: void Render(gfx::Canvas* canvas) const;

  DISALLOW_COPY_AND_ASSIGN(Renderer);
};

namespace {
std::unique_ptr<gfx::TextLayout> CreateTextLayout(const base::string16& text,
    const LabelControl::Style& style, const gfx::SizeF& size) {
  gfx::TextFormat text_format(style.font_family, style.font_size);
  return text_format.CreateLayout(text, size);
}
}  // namespace

LabelControl::Renderer::Renderer(const base::string16& text,
                                 const Style& style,
                                 const gfx::RectF& rect)
    : bounds_(rect), style_(style),
      text_layout_(CreateTextLayout(text, style, rect.size())) {
  // TODO(yosi) We should share following code fragment with |ButtonControl|
  // and |TextFieldControl|.
  DWRITE_TEXT_METRICS metrics;
  COM_VERIFY((*text_layout_)->GetMetrics(&metrics));
  auto const text_size = gfx::SizeF(metrics.width, metrics.height);
  // Render text at middle of control.
  auto const offset = (bounds_.size() - text_size) / 2.0f;
  text_origin_ = gfx::PointF(bounds_.left, bounds_.top + offset.height);
}

LabelControl::Renderer::~Renderer() {
}

void LabelControl::Renderer::Render(gfx::Canvas* canvas) const {
  gfx::Canvas::AxisAlignedClipScope clip_scope(canvas, bounds_);
  canvas->FillRectangle(gfx::Brush(canvas, style_.bgcolor), bounds_);
  gfx::Brush text_brush(canvas, style_.color);
  (*canvas)->DrawTextLayout(text_origin_, *text_layout_, text_brush,
                            D2D1_DRAW_TEXT_OPTIONS_CLIP);
  canvas->Flush();
}

//////////////////////////////////////////////////////////////////////
//
// LabelControl
//
LabelControl::LabelControl(ControlController* controller,
                           const base::string16& text, const Style& style)
    : Control(controller), style_(style), text_(text) {
}

LabelControl::~LabelControl() {
}

bool LabelControl::focusable() const {
  return false;
}

void LabelControl::set_style(const Style& new_style) {
  if (style_ == new_style)
    return;
  style_ = new_style;
  renderer_.reset();
}

void LabelControl::set_text(const base::string16& new_text) {
  if (text_ == new_text)
    return;
  text_ = new_text;
  renderer_.reset();
}

// ui::Widget
void LabelControl::DidChangeBounds() {
  renderer_.reset();
}

void LabelControl::OnDraw(gfx::Canvas* canvas) {
  if (!renderer_)
    renderer_ = std::make_unique<Renderer>(text_, style_, gfx::RectF(bounds()));
  renderer_->Render(canvas);
}

}  // namespace ui
