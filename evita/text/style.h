// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_text_style_h)
#define INCLUDE_evita_text_style_h

#include <functional>

#include "base/strings/string16.h"

namespace text {

// Color
class Color {
  COLORREF    m_cr;

  public: Color(COLORREF cr = 0);
  public: Color(int r, int g, int b);
  public: ~Color();

  public: operator COLORREF() const { return m_cr; }

  public: bool operator==(const Color& other) const;
  public: bool operator!=(const Color& other) const;

  public: bool Equal(const Color& cr) const { return m_cr == cr.m_cr; }
  public: size_t Hash() const { return m_cr; }
};

typedef int FontSize;

enum FontStyle {
  FontStyle_Normal,
  FontStyle_Italic,
};

enum FontWeight {
  FontWeight_Normal,
  FontWeight_Bold,
};

enum TextDecoration {
  TextDecoration_None,
  TextDecoration_GreenWave,
  TextDecoration_RedWave,
  TextDecoration_Underline,
  #if SUPPORT_IME
  TextDecoration_ImeInput,        // dotted underline
  TextDecoration_ImeActive,       // 2 pixel underline
  TextDecoration_ImeInactiveA,    // 1 pixel underline
  TextDecoration_ImeInactiveB,    // 1 pixel underline
  #endif // SUPPORT_IME
};

typedef int Syntax;

//////////////////////////////////////////////////////////////////////
//
// Style
//
class Style {
  private: enum Mask {
    Mask_BgColor = 1 << 0,
    Mask_Color      = 1 << 1,
    Mask_FontFamily = 1 << 2,
    Mask_FontSize   = 1 << 3,
    Mask_FontStyle  = 1 << 4,
    Mask_FontWeight = 1 << 5,
    Mask_MarkerColor = 1 << 6,
    Mask_Syntax     = 1 << 7,
    Mask_TextDecoration = 1 << 8,
  };

  private: Color bgcolor_;
  private: Color color_;
  private: base::string16 font_family_;
  private: int font_size_;
  private: FontStyle font_style_;
  private: FontWeight font_weight_;
  private: Color marker_color_;
  private: int masks_;
  private: int syntax_;
  private: TextDecoration text_decoration_;

  public: Style(Color color, Color bgcolor);
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

  public: Syntax syntax() const;
  public: bool has_syntax() const { return masks_ & Mask_Syntax; }
  public: void set_syntax(Syntax syntax);

  public: TextDecoration text_decoration() const;
  public: bool has_text_decoration() const {
    return masks_ & Mask_TextDecoration;
  }
  public: void set_text_decoration(TextDecoration text_decoration);

  public: static Style* Default();
  public: void Merge(const Style& style);
  public: void OverrideBy(const Style& style);
};

}  // namespace text

namespace std {
template<> struct hash<text::Color> {
  size_t operator()(const text::Color& color) const;
};

template<> struct hash<text::Style> {
  size_t operator()(const text::Style& style) const;
};
}  // namespace std

#endif //!defined(INCLUDE_evita_text_style_h)
