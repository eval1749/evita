// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_STYLE_COMPUTED_STYLE_H_
#define EVITA_TEXT_STYLE_COMPUTED_STYLE_H_

#include <vector>

#include "base/strings/string16.h"
#include "evita/gfx/color_f.h"

namespace gfx {
class Font;
}

namespace layout {

enum class TextDecorationLine {
  None,
  LineThrough,
  Overline,
  Underline,
};

enum class TextDecorationStyle {
  Dashed,
  Dotted,
  Double,
  Solid,
  Wavy,
};

//////////////////////////////////////////////////////////////////////
//
// ComputedStyle
//
class ComputedStyle {
 public:
  class Builder;

  ComputedStyle(const ComputedStyle& other) = delete;
  ComputedStyle(ComputedStyle&& other);
  ~ComputedStyle();

  ComputedStyle& operator=(const ComputedStyle& other) = delete;
  ComputedStyle& operator=(ComputedStyle&& other) = delete;

  // For inline box cache
  bool operator==(const ComputedStyle& other) const = delete;
  bool operator!=(const ComputedStyle& other) const = delete;

  const gfx::ColorF& bgcolor() const { return bgcolor_; }
  const gfx::ColorF& color() const { return color_; }
  const std::vector<const gfx::Font*>& fonts() const { return fonts_; }
  const gfx::ColorF& text_decoration_color() const {
    return text_decoration_color_;
  }
  TextDecorationLine text_decoration_line() const {
    return text_decoration_line_;
  }
  TextDecorationStyle text_decoration_style() const {
    return text_decoration_style_;
  }

 private:
  ComputedStyle();

  gfx::ColorF bgcolor_;
  gfx::ColorF color_;
  std::vector<const gfx::Font*> fonts_;
  gfx::ColorF text_decoration_color_;
  TextDecorationLine text_decoration_line_ = TextDecorationLine::None;
  TextDecorationStyle text_decoration_style_ = TextDecorationStyle::Solid;
};

}  // namespace layout

#endif  // EVITA_TEXT_STYLE_COMPUTED_STYLE_H_
