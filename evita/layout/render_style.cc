// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/layout/render_style.h"

#include "evita/layout/render_font.h"

namespace views {
namespace rendering {

gfx::ColorF ColorToColorF(const css::Color& color) {
  return gfx::ColorF(static_cast<float>(color.red()) / 255,
                     static_cast<float>(color.green()) / 255,
                     static_cast<float>(color.blue()) / 255, color.alpha());
}

RenderStyle::RenderStyle(const css::Style& values, const Font* font)
    : bgcolor_(ColorToColorF(values.bgcolor())),
      color_(ColorToColorF(values.color())),
      font_(font),
      text_decoration_(values.text_decoration()) {}

RenderStyle::RenderStyle(const css::Style& values, const Font& font)
    : RenderStyle(values, &font) {}

RenderStyle::RenderStyle(const RenderStyle& other)
    : bgcolor_(other.bgcolor_),
      color_(other.color_),
      font_(other.font_),
      overlay_color_(other.overlay_color_),
      text_decoration_(other.text_decoration_) {}

RenderStyle::RenderStyle()
    : font_(nullptr), text_decoration_(css::TextDecoration::None) {}

RenderStyle::~RenderStyle() {}

bool RenderStyle::operator==(const RenderStyle& other) const {
  return bgcolor_ == other.bgcolor_ && color_ == other.color_ &&
         font_ == other.font_ && overlay_color_ == other.overlay_color_ &&
         text_decoration_ == other.text_decoration_;
}

bool RenderStyle::operator!=(const RenderStyle& other) const {
  return !operator==(other);
}

void RenderStyle::set_overlay_color(const css::Color& color) {
  overlay_color_ = ColorToColorF(color);
}

}  // namespace rendering
}  // namespace views

namespace std {
size_t hash<views::rendering::RenderStyle>::operator()(
    const views::rendering::RenderStyle& style) const {
  auto result = static_cast<size_t>(0);
  result ^= std::hash<gfx::ColorF>()(style.bgcolor());
  result ^= std::hash<gfx::ColorF>()(style.color());
  result ^= std::hash<views::rendering::Font>()(style.font());
  result ^= std::hash<gfx::ColorF>()(style.overlay_color());
  result ^= std::hash<css::TextDecoration>()(style.text_decoration());
  return result;
}
}  // namespace std
