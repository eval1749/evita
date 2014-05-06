// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_css_style_h)
#define INCLUDE_evita_css_style_h

#include <functional>

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
  #if SUPPORT_IME
  ImeInput, // dotted underline
  ImeActive, // 2 pixel underline
  ImeInactiveA, // 1 pixel underline
  ImeInactiveB, // 1 pixel underline
  #endif // SUPPORT_IME
};

//////////////////////////////////////////////////////////////////////
//
// Style
//
class Style {
  private: enum Mask {
    Mask_BgColor = 1 << 0,
    Mask_Color = 1 << 1,
    Mask_FontFamily = 1 << 2,
    Mask_FontSize = 1 << 3,
    Mask_FontStyle = 1 << 4,
    Mask_FontWeight = 1 << 5,
    Mask_MarkerColor = 1 << 6,
    Mask_TextDecoration = 1 << 7,
  };

  private: Color bgcolor_;
  private: Color color_;
  private: base::string16 font_family_;
  private: FontSize font_size_;
  private: FontStyle font_style_;
  private: FontWeight font_weight_;
  private: Color marker_color_;
  private: int masks_;
  private: TextDecoration text_decoration_;

  public: Style(const Color& color, const Color& bgcolor);
  public: Style(const Style& other);
  public: Style();
  public: ~Style();

  public: bool operator==(const Style& other) const;
  public: bool operator!=(const Style& other) const;

  public: Color bgcolor() const;
  public: bool has_bgcolor() const { return masks_ & Mask_BgColor; }
  public: void set_bgcolor(Color bgcolor);

  public: Color color() const;
  public: bool has_color() const { return masks_ & Mask_Color; }
  public: void set_color(Color color);

  public: const base::string16& font_family() const;
  public: bool has_font_family() const { return masks_ & Mask_FontFamily; }
  public: void set_font_family(const base::string16& font_family);

  public: FontSize font_size() const;
  public: bool has_font_size() const { return masks_ & Mask_FontSize; }
  public: void set_font_size(FontSize font_size);

  public: FontStyle font_style() const;
  public: bool has_font_style() const { return masks_ & Mask_FontStyle; }
  public: void set_font_style(FontStyle font_style);

  public: FontWeight font_weight() const;
  public: bool has_font_weight() const { return masks_ & Mask_FontWeight; }
  public: void set_font_weight(FontWeight font_weight);

  public: Color marker_color() const;
  public: bool has_marker_color() const { return masks_ & Mask_MarkerColor; }
  public: void set_marker_color(Color marker_color);

  public: TextDecoration text_decoration() const;
  public: bool has_text_decoration() const {
    return masks_ & Mask_TextDecoration;
  }
  public: void set_text_decoration(TextDecoration text_decoration);

  public: static Style* Default();
  public: void Merge(const Style& style);
  public: void OverrideBy(const Style& style);
};

}  // namespace css

namespace std {
template<> struct hash<css::Style> {
  size_t operator()(const css::Style& style) const;
};
}  // namespace std

#endif //!defined(INCLUDE_evita_css_style_h)
