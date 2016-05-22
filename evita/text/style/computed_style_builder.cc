// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/style/computed_style_builder.h"

namespace layout {

using Builder = ComputedStyle::Builder;

Builder::Builder() = default;
Builder::~Builder() = default;

ComputedStyle Builder::Build() {
  return style_;
}

Builder& Builder::Load(const css::Style& model, const gfx::Font& font) {
  SetBackgroundColor(ColorToColorF(model.bgcolor()));
  SetColor(ColorToColorF(model.color()));
  SetFont(font);
  if (model.has_text_decoration_color())
    SetTextDecorationColor(ColorToColorF(model.text_decoration_color()));
  else
    SetTextDecorationColor(style_.color_);
  if (model.has_text_decoration_line())
    SetTextDecorationLine(model.text_decoration_line());
  if (model.has_text_decoration_style())
    SetTextDecorationStyle(model.text_decoration_style());
  return *this;
}

Builder& Builder::SetBackgroundColor(const gfx::ColorF& color) {
  style_.bgcolor_ = color;
  return *this;
}

Builder& Builder::SetColor(const gfx::ColorF& color) {
  style_.color_ = color;
  return *this;
}

Builder& Builder::SetFont(const gfx::Font& font) {
  style_.font_ = &font;
  return *this;
}

Builder& Builder::SetTextDecorationColor(const gfx::ColorF& color) {
  style_.text_decoration_color_ = color;
  return *this;
}

Builder& Builder::SetTextDecorationLine(css::TextDecorationLine line) {
  style_.text_decoration_line_ = line;
  return *this;
}

Builder& Builder::SetTextDecorationStyle(css::TextDecorationStyle style) {
  style_.text_decoration_style_ = style;
  return *this;
}

}  // namespace layout
