// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/style/computed_style.h"

#include "evita/gfx/font.h"

namespace layout {

gfx::ColorF ColorToColorF(const css::Color& color) {
  return gfx::ColorF(static_cast<float>(color.red()) / 255,
                     static_cast<float>(color.green()) / 255,
                     static_cast<float>(color.blue()) / 255, color.alpha());
}

ComputedStyle::ComputedStyle(const ComputedStyle& other)
    : bgcolor_(other.bgcolor_),
      color_(other.color_),
      font_(other.font_),
      text_decoration_(other.text_decoration_) {}

ComputedStyle::ComputedStyle()
    : font_(nullptr), text_decoration_(css::TextDecoration::None) {}

ComputedStyle::~ComputedStyle() {}

bool ComputedStyle::operator==(const ComputedStyle& other) const {
  return bgcolor_ == other.bgcolor_ && color_ == other.color_ &&
         font_ == other.font_ && text_decoration_ == other.text_decoration_;
}

bool ComputedStyle::operator!=(const ComputedStyle& other) const {
  return !operator==(other);
}

}  // namespace layout

namespace std {
size_t hash<layout::ComputedStyle>::operator()(
    const layout::ComputedStyle& style) const {
  auto result = static_cast<size_t>(0);
  result ^= std::hash<gfx::ColorF>()(style.bgcolor());
  result ^= std::hash<gfx::ColorF>()(style.color());
  result ^= std::hash<gfx::Font>()(style.font());
  result ^= std::hash<css::TextDecoration>()(style.text_decoration());
  return result;
}
}  // namespace std
