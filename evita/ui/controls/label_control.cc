// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/controls/label_control.h"

#include "evita/gfx_base.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// LabelControl::LabelStyle
//
bool LabelControl::LabelStyle::operator==(const LabelStyle& other) const {
  return bgcolor == other.bgcolor && color == other.color &&
         font_family == other.font_family && font_size == other.font_size;
}

bool LabelControl::LabelStyle::operator!=(const LabelStyle& other) const {
  return !operator==(other);
}

//////////////////////////////////////////////////////////////////////
//
// LabelControl::Renderer
//
class LabelControl::Renderer {
  private: gfx::RectF rect_;
  private: LabelStyle style_;
  private: std::unique_ptr<gfx::TextLayout> text_layout_;

  public: Renderer(const base::string16& text, const LabelStyle& style,
                   const gfx::RectF& rect);
  public: ~Renderer();

  public: void Render(gfx::Graphics* gfx) const;

  DISALLOW_COPY_AND_ASSIGN(Renderer);
};

namespace {
std::unique_ptr<gfx::TextLayout> CreateTextLayout(const base::string16& text,
    const LabelControl::LabelStyle& style, const gfx::SizeF& size) {
  gfx::TextFormat text_format(style.font_family, style.font_size);
  return text_format.CreateLayout(text, size);
}
}  // namespace

LabelControl::Renderer::Renderer(const base::string16& text,
                                 const LabelStyle& style,
                                 const gfx::RectF& rect)
    : rect_(rect), style_(style),
      text_layout_(CreateTextLayout(text, style, rect.size())) {
}

LabelControl::Renderer::~Renderer() {
}

void LabelControl::Renderer::Render(gfx::Graphics* gfx) const {
  gfx->FillRectangle(gfx::Brush(*gfx, style_.bgcolor), rect_);

  gfx::Brush text_brush(*gfx, style_.color);
  (*gfx)->DrawTextLayout(rect_.left_top(), *text_layout_, text_brush,
                         D2D1_DRAW_TEXT_OPTIONS_CLIP);
  gfx->Flush();
}

//////////////////////////////////////////////////////////////////////
//
// LabelControl
//
LabelControl::LabelControl(ControlController* controller,
                           const base::string16& text, const LabelStyle& style)
    : Control(controller), style_(style), text_(text) {
}

LabelControl::~LabelControl() {
}

bool LabelControl::focusable() const {
  return false;
}

void LabelControl::set_style(const LabelStyle& new_style) {
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
void LabelControl::DidResize() {
  renderer_.reset();
}

void LabelControl::OnDraw(gfx::Graphics* gfx) {
  if (!renderer_)
    renderer_ = std::make_unique<Renderer>(text_, style_, gfx::RectF(rect()));
  renderer_->Render(gfx);
}

}  // namespace ui
