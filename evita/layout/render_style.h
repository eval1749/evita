// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_RENDER_STYLE_H_
#define EVITA_LAYOUT_RENDER_STYLE_H_

#include <functional>

#include "base/basictypes.h"
#include "evita/gfx_base.h"
#include "evita/css/style.h"

namespace gfx {
class Font;
}

namespace layout {

//////////////////////////////////////////////////////////////////////
//
// RenderStyle
//
class RenderStyle {
 public:
  RenderStyle(const css::Style& values, const gfx::Font& font);
  RenderStyle(const css::Style& values, const gfx::Font* font);
  RenderStyle(const RenderStyle& other);
  RenderStyle();
  ~RenderStyle();

  bool operator==(const RenderStyle& other) const;
  bool operator!=(const RenderStyle& other) const;

  const gfx::ColorF& bgcolor() const { return bgcolor_; }
  const gfx::ColorF& color() const { return color_; }
  const gfx::Font& font() const { return *font_; }
  void set_fond(const gfx::Font& font) { font_ = &font; }
  const gfx::ColorF& overlay_color() const { return overlay_color_; }
  void set_overlay_color(const css::Color& color);
  css::TextDecoration text_decoration() const { return text_decoration_; }

 private:
  gfx::ColorF bgcolor_;
  gfx::ColorF color_;
  const gfx::Font* font_;
  gfx::ColorF overlay_color_;
  css::TextDecoration text_decoration_;
};

gfx::ColorF ColorToColorF(const css::Color& color);

}  // namespace layout

namespace std {
template <>
struct hash<layout::RenderStyle> {
  size_t operator()(const layout::RenderStyle& style) const;
};
}  // namespace std

#endif  // EVITA_LAYOUT_RENDER_STYLE_H_
