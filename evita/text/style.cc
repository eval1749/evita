// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/style.h"

#include "base/logging.h"

namespace text {

//////////////////////////////////////////////////////////////////////
//
// Color
//
Color::Color(COLORREF cr) : m_cr(cr) {
}

Color::Color(int r, int g, int b) : m_cr(RGB(r, g, b)) {
}

Color::~Color() {
}

//////////////////////////////////////////////////////////////////////
//
// StyleValues
//
StyleValues::StyleValues(Color color, Color bgcolor)
    : StyleValues() {
  set_color(color);
  set_bgcolor(bgcolor);
}

StyleValues::StyleValues(const StyleValues& other)
    : bgcolor_(other.bgcolor_),
      color_(other.color_),
      font_family_(other.font_family_),
      font_size_(other.font_size_),
      font_style_(other.font_style_),
      font_weight_(other.font_weight_),
      marker_color_(other.marker_color_),
      masks_(other.masks_),
      syntax_(other.syntax_),
      text_decoration_(other.text_decoration_) {
}

StyleValues::StyleValues() : masks_(0) {
}

StyleValues::~StyleValues() {
}

bool StyleValues::operator==(const StyleValues& other) const {
  if (masks_ != other.masks_)
    return false;
  if (has_bgcolor() && bgcolor_ != other.bgcolor_)
    return false;
  if (has_color() && color_ != other.color_)
    return false;
  if (has_font_size() && font_size_ != other.font_size_)
    return false;
  if (has_font_style() && font_style_ != other.font_style_)
    return false;
  if (has_font_weight() && font_weight_ != other.font_weight_)
    return false;
  if (has_marker_color() && marker_color_ != other.marker_color_)
    return false;
  if (has_syntax() && syntax_ != other.syntax_)
    return false;
  if (has_text_decoration() && text_decoration_ != other.text_decoration_)
    return false;
  return !has_font_family() || font_family_ == other.font_family_;
}

bool StyleValues::operator!=(const StyleValues& other) const {
  return !operator==(other);
}

Color StyleValues::bgcolor() const {
  DCHECK(has_bgcolor());
  return bgcolor_;
}

void StyleValues::set_bgcolor(Color color) {
  bgcolor_ = color;
  masks_ |= Mask_BgColor;
}

Color StyleValues::color() const {
  DCHECK(has_color());
  return color_;
}

void StyleValues::set_color(Color color) {
  color_ = color;
  masks_ |= Mask_Color;
}

const base::string16& StyleValues::font_family() const {
  DCHECK(has_font_family());
  return font_family_;
}

void StyleValues::set_font_family(const base::string16& font_family) {
  font_family_ = font_family;
  masks_ |= Mask_FontFamily;
}

FontSize StyleValues::font_size() const {
  DCHECK(has_font_size());
  return font_size_;
}

void StyleValues::set_font_size(FontSize font_size) {
  font_size_ = font_size;
  masks_ |= Mask_FontSize;
}

FontStyle StyleValues::font_style() const {
  DCHECK(has_font_style());
  return font_style_;
}

void StyleValues::set_font_style(FontStyle font_style) {
  font_style_ = font_style;
  masks_ |= Mask_FontStyle;
}

FontWeight StyleValues::font_weight() const {
  DCHECK(has_font_weight());
  return font_weight_;
}

void StyleValues::set_font_weight(FontWeight font_weight) {
  font_weight_ = font_weight;
  masks_ |= Mask_FontWeight;
}

Color StyleValues::marker_color() const {
  DCHECK(has_marker_color());
  return marker_color_;
}

void StyleValues::set_marker_color(Color color) {
  marker_color_ = color;
  masks_ |= Mask_MarkerColor;
}

Syntax StyleValues::syntax() const {
  DCHECK(has_syntax());
  return syntax_;
}

void StyleValues::set_syntax(Syntax syntax) {
  syntax_ = syntax;
  masks_ |= Mask_Syntax;
}

TextDecoration StyleValues::text_decoration() const {
  DCHECK(has_text_decoration());
  return text_decoration_;
}

void StyleValues::set_text_decoration(
    TextDecoration text_decoration) {
  text_decoration_ = text_decoration;
  masks_ |= Mask_TextDecoration;
}

StyleValues* StyleValues::Default() {
  static bool init;
  DEFINE_STATIC_LOCAL(StyleValues, default_style, ());
  if (!init) {
    init = true;
    #if 0
        //default_style.SetBackground(Color(0xF0, 0xF0, 0xF0));
        default_style.set_bgcolor(Color(247, 247, 239));
        default_style.set_color(Color(0x00, 0x00, 0x00));
        default_style.Set_marker_color{r(Color(0x00, 0x66, 0x00));
    #else
        default_style.set_bgcolor(Color(255, 255, 255));
        default_style.set_color(Color(0x00, 0x00, 0x00));
        default_style.set_marker_color(Color(0x00, 0x99, 0x00));
    #endif

    default_style.set_font_family(L"Consolas, MS Gothic");
    default_style.set_font_size(10);
    default_style.set_font_style(FontStyle_Normal);
    default_style.set_font_weight(FontWeight_Normal);
    default_style.set_syntax(0);
    default_style.set_text_decoration(TextDecoration_None);
  }

  return &default_style;
}

void StyleValues::OverrideBy(const StyleValues& other) {
  if (other.has_bgcolor()) {
    set_bgcolor(other.bgcolor());
  }
  if (other.has_color()) {
    set_color(other.color());
  }
  if (other.has_font_family()) {
    set_font_family(other.font_family());
  }
  if (other.has_font_size()) {
    set_font_size(other.font_size());
  }
  if (other.has_font_style()) {
    set_font_style(other.font_style());
  }
  if (other.has_font_weight()) {
    set_font_weight(other.font_weight());
  }
  if (other.has_marker_color()) {
    set_marker_color(other.marker_color());
  }
  if (other.has_syntax()) {
    set_syntax(other.syntax());
  }
  if (other.has_text_decoration()) {
    set_text_decoration(other.text_decoration());
  }
}

}  // namspace text
