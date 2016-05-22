// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/style/computed_style_builder.h"

#include "evita/text/style/computed_style.h"

namespace layout {

using Builder = ComputedStyle::Builder;

Builder::Builder() : style_(new ComputedStyle()) {}
Builder::~Builder() = default;

std::unique_ptr<ComputedStyle> Builder::Build() {
  return std::move(style_);
}

Builder& Builder::SetBackgroundColor(const gfx::ColorF& color) {
  style_->bgcolor_ = color;
  return *this;
}

Builder& Builder::SetColor(const gfx::ColorF& color) {
  style_->color_ = color;
  return *this;
}

Builder& Builder::SetFonts(const std::vector<const gfx::Font*>& fonts) {
  style_->fonts_ = fonts;
  return *this;
}

Builder& Builder::SetTextDecorationColor(const gfx::ColorF& color) {
  style_->text_decoration_color_ = color;
  return *this;
}

Builder& Builder::SetTextDecorationLine(TextDecorationLine line) {
  style_->text_decoration_line_ = line;
  return *this;
}

Builder& Builder::SetTextDecorationStyle(TextDecorationStyle style) {
  style_->text_decoration_style_ = style;
  return *this;
}

}  // namespace layout
