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
  SetTextDecoration(model.text_decoration());
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

Builder& Builder::SetTextDecoration(css::TextDecoration decoration) {
  style_.text_decoration_ = decoration;
  return *this;
}

}  // namespace layout
