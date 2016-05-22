// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_STYLE_COMPUTED_STYLE_H_
#define EVITA_TEXT_STYLE_COMPUTED_STYLE_H_

#include <functional>

#include "base/macros.h"
#include "evita/gfx/color_f.h"
#include "evita/text/style/models/style.h"

namespace gfx {
class Font;
}

namespace layout {

//////////////////////////////////////////////////////////////////////
//
// ComputedStyle
//
class ComputedStyle {
 public:
  class Builder;

  ComputedStyle(const ComputedStyle& other);
  ~ComputedStyle();

  bool operator==(const ComputedStyle& other) const;
  bool operator!=(const ComputedStyle& other) const;

  const gfx::ColorF& bgcolor() const { return bgcolor_; }
  const gfx::ColorF& color() const { return color_; }
  const gfx::Font& font() const { return *font_; }
  void set_fond(const gfx::Font& font) { font_ = &font; }
  const gfx::ColorF& text_decoration_color() const {
    return text_decoration_color_;
  }
  xcss::TextDecorationLine text_decoration_line() const {
    return text_decoration_line_;
  }
  xcss::TextDecorationStyle text_decoration_style() const {
    return text_decoration_style_;
  }

 private:
  ComputedStyle();

  gfx::ColorF bgcolor_;
  gfx::ColorF color_;
  const gfx::Font* font_;
  gfx::ColorF text_decoration_color_;
  xcss::TextDecorationLine text_decoration_line_;
  xcss::TextDecorationStyle text_decoration_style_;
};

gfx::ColorF ColorToColorF(const xcss::Color& color);

}  // namespace layout

namespace std {
template <>
struct hash<layout::ComputedStyle> {
  size_t operator()(const layout::ComputedStyle& style) const;
};
}  // namespace std

#endif  // EVITA_TEXT_STYLE_COMPUTED_STYLE_H_
