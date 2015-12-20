// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_CSS_STYLE_H_
#define EVITA_CSS_STYLE_H_

#include <functional>
#include <vector>

#include "base/strings/string16.h"
#include "evita/css/color.h"

namespace css {

typedef float FontSize;

enum class FontStyle {
  Normal,
  Italic,
};

enum class FontWeight {
  Normal,
  Bold,
};

enum class TextDecoration {
  None,
  GreenWave,
  RedWave,
  Underline,
  ImeInput,      // dotted underline
  ImeActive,     // 2 pixel underline
  ImeInactiveA,  // 1 pixel underline
  ImeInactiveB,  // 1 pixel underline
};

// name, CapitalName, type
#define FOR_EACH_CSS_PROPERTY(V)      \
  V(bgcolor, BgColor, Color)          \
  V(color, Color, Color)              \
  V(font_family, FontFamily)          \
  V(font_size, FontSize)              \
  V(font_style, FontStyle)            \
  V(font_weight, FontWeight)          \
  V(marker_color, MarkerColor, Color) \
  V(text_decoration, TextDecoration, TextDecoration)

//////////////////////////////////////////////////////////////////////
//
// Style
//
class Style final {
 public:
  Style(const Color& color, const Color& bgcolor);
  Style(const Style& other);
  Style();
  ~Style();

  bool operator==(const Style& other) const;
  bool operator!=(const Style& other) const;

  Color bgcolor() const;
  bool has_bgcolor() const { return (masks_ & Mask_BgColor) != 0; }
  void set_bgcolor(Color bgcolor);

  Color color() const;
  bool has_color() const { return (masks_ & Mask_Color) != 0; }
  void set_color(Color color);

  const std::vector<base::string16>& font_families() const;
  const base::string16& font_family() const;
  bool has_font_family() const { return (masks_ & Mask_FontFamily) != 0; }
  void set_font_family(const base::string16& font_family);

  FontSize font_size() const;
  bool has_font_size() const { return (masks_ & Mask_FontSize) != 0; }
  void set_font_size(FontSize font_size);

  FontStyle font_style() const;
  bool has_font_style() const { return (masks_ & Mask_FontStyle) != 0; }
  void set_font_style(FontStyle font_style);

  FontWeight font_weight() const;
  bool has_font_weight() const { return (masks_ & Mask_FontWeight) != 0; }
  void set_font_weight(FontWeight font_weight);

  Color marker_color() const;
  bool has_marker_color() const { return (masks_ & Mask_MarkerColor) != 0; }
  void set_marker_color(Color marker_color);

  TextDecoration text_decoration() const;
  bool has_text_decoration() const {
    return (masks_ & Mask_TextDecoration) != 0;
  }
  void set_text_decoration(TextDecoration text_decoration);

  static Style* Default();

  bool IsSubsetOf(const Style& other) const;
  void Merge(const Style& style);
  void OverrideBy(const Style& style);

 private:
  enum Mask {
    Mask_BgColor = 1 << 0,
    Mask_Color = 1 << 1,
    Mask_FontFamily = 1 << 2,
    Mask_FontSize = 1 << 3,
    Mask_FontStyle = 1 << 4,
    Mask_FontWeight = 1 << 5,
    Mask_MarkerColor = 1 << 6,
    Mask_TextDecoration = 1 << 7,
  };

  Color bgcolor_;
  Color color_;
  mutable std::vector<base::string16> font_families_;
  base::string16 font_family_;
  FontSize font_size_;
  FontStyle font_style_;
  FontWeight font_weight_;
  Color marker_color_;
  int masks_;
  TextDecoration text_decoration_;
  void Prepare() const;
};

}  // namespace css

namespace std {
template <>
struct hash<css::Style> {
  size_t operator()(const css::Style& style) const;
};
}  // namespace std

#endif  // EVITA_CSS_STYLE_H_
