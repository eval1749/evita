// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/css/style.h"

#include "base/logging.h"
#include "base/strings/string_util.h"

namespace css {

namespace {
std::vector<base::string16> parseFontFamily(const base::string16& source) {
  enum class State {
    Comma,
    Name,
    NameSpace,
    Start,
  } state = State::Start;
  std::vector<base::string16> names;
  base::string16 name;
  for (auto const ch : source) {
    switch (state) {
      case State::Name:
        if (ch == ',') {
          names.push_back(name);
          name.clear();
          state = State::Start;
          break;
        }
        if (base::IsAsciiWhitespace(ch))
          state = State::NameSpace;
        else
          name.push_back(ch);
        break;
      case State::NameSpace:
        if (ch == ',') {
          names.push_back(name);
          name.clear();
          state = State::Start;
          break;
        }
        if (!base::IsAsciiWhitespace(ch)) {
          name.push_back(' ');
          name.push_back(ch);
          state = State::Name;
        }
        break;
      case State::Start:
        if (ch != ',' && !base::IsAsciiWhitespace(ch)) {
          name.push_back(ch);
          state = State::Name;
        }
        break;
    }
  }
  if (name.length())
    names.push_back(name);
  return names;
}
}  // namespace

Style::Style(const Color& color, const Color& bgcolor) : Style() {
  set_color(color);
  set_bgcolor(bgcolor);
}

Style::Style(const Style& other)
    : bgcolor_(other.bgcolor_),
      color_(other.color_),
      font_families_(other.font_families_),
      font_family_(other.font_family_),
      font_size_(other.font_size_),
      font_style_(other.font_style_),
      font_weight_(other.font_weight_),
      marker_color_(other.marker_color_),
      masks_(other.masks_),
      text_decoration_(other.text_decoration_) {}

Style::Style() : masks_(0) {}

Style::~Style() {}

bool Style::operator==(const Style& other) const {
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
  if (has_text_decoration() && text_decoration_ != other.text_decoration_)
    return false;
  return !has_font_family() || font_family_ == other.font_family_;
}

bool Style::operator!=(const Style& other) const {
  return !operator==(other);
}

Color Style::bgcolor() const {
  DCHECK(has_bgcolor());
  return bgcolor_;
}

void Style::set_bgcolor(Color color) {
  bgcolor_ = color;
  masks_ |= Mask_BgColor;
}

Color Style::color() const {
  DCHECK(has_color());
  return color_;
}

void Style::set_color(Color color) {
  color_ = color;
  masks_ |= Mask_Color;
}

const std::vector<base::string16>& Style::font_families() const {
  if (has_font_family() && font_families_.empty())
    font_families_ = parseFontFamily(font_family_);
  return font_families_;
}

const base::string16& Style::font_family() const {
  DCHECK(has_font_family());
  return font_family_;
}

void Style::set_font_family(const base::string16& font_family) {
  font_family_ = font_family;
  masks_ |= Mask_FontFamily;
  font_families_.clear();
}

FontSize Style::font_size() const {
  DCHECK(has_font_size());
  return font_size_;
}

void Style::set_font_size(FontSize font_size) {
  font_size_ = font_size;
  masks_ |= Mask_FontSize;
}

FontStyle Style::font_style() const {
  DCHECK(has_font_style());
  return font_style_;
}

void Style::set_font_style(FontStyle font_style) {
  font_style_ = font_style;
  masks_ |= Mask_FontStyle;
}

FontWeight Style::font_weight() const {
  DCHECK(has_font_weight());
  return font_weight_;
}

void Style::set_font_weight(FontWeight font_weight) {
  font_weight_ = font_weight;
  masks_ |= Mask_FontWeight;
}

Color Style::marker_color() const {
  DCHECK(has_marker_color());
  return marker_color_;
}

void Style::set_marker_color(Color color) {
  marker_color_ = color;
  masks_ |= Mask_MarkerColor;
}

TextDecoration Style::text_decoration() const {
  DCHECK(has_text_decoration());
  return text_decoration_;
}

void Style::set_text_decoration(TextDecoration text_decoration) {
  text_decoration_ = text_decoration;
  masks_ |= Mask_TextDecoration;
}

Style* Style::Default() {
  static bool init;
  CR_DEFINE_STATIC_LOCAL(Style, default_style, ());
  if (!init) {
    init = true;
#if 0
    //  default_style.SetBackground(Color(0xF0, 0xF0, 0xF0));
    default_style.set_bgcolor(Color(247, 247, 239));
    default_style.set_color(Color(0x00, 0x00, 0x00));
    default_style.Set_marker_color(Color(0x00, 0x66, 0x00));
#else
    default_style.set_bgcolor(Color(255, 255, 255));
    default_style.set_color(Color(0x00, 0x00, 0x00));
    default_style.set_marker_color(Color(0x00, 0x99, 0x00));
#endif

    default_style.set_font_family(L"Consolas, Meiryo");
    default_style.set_font_size(10);
    default_style.set_font_style(FontStyle::Normal);
    default_style.set_font_weight(FontWeight::Normal);
    default_style.set_text_decoration(TextDecoration::None);
    default_style.Prepare();
  }

  return &default_style;
}

void Style::Merge(const Style& other) {
  other.Prepare();
  if (!has_bgcolor() && other.has_bgcolor())
    set_bgcolor(other.bgcolor());
  if (!has_color() && other.has_color())
    set_color(other.color());
  if (!has_font_family() && other.has_font_family()) {
    set_font_family(other.font_family());
    font_families_ = other.font_families_;
  }
  if (!has_font_size() && other.has_font_size())
    set_font_size(other.font_size());
  if (!has_font_style() && other.has_font_style())
    set_font_style(other.font_style());
  if (!has_font_weight() && other.has_font_weight())
    set_font_weight(other.font_weight());
  if (!has_marker_color() && other.has_marker_color())
    set_marker_color(other.marker_color());
  if (!has_text_decoration() && other.has_text_decoration())
    set_text_decoration(other.text_decoration());
}

void Style::OverrideBy(const Style& other) {
  other.Prepare();
  if (other.has_bgcolor())
    set_bgcolor(other.bgcolor());
  if (other.has_color())
    set_color(other.color());
  if (other.has_font_family()) {
    set_font_family(other.font_family());
    font_families_ = other.font_families_;
  }
  if (other.has_font_size())
    set_font_size(other.font_size());
  if (other.has_font_style())
    set_font_style(other.font_style());
  if (other.has_font_weight())
    set_font_weight(other.font_weight());
  if (other.has_marker_color())
    set_marker_color(other.marker_color());
  if (other.has_text_decoration())
    set_text_decoration(other.text_decoration());
}

void Style::Prepare() const {
  if (!has_font_family() || !font_families_.empty())
    return;
  font_families_ = parseFontFamily(font_family_);
}

}  // namespace css

namespace std {
size_t hash<css::Style>::operator()(const css::Style& style) const {
  size_t result = 137u;
  if (style.has_bgcolor()) {
    result <<= 1;
    result ^= std::hash<css::Color>()(style.bgcolor());
  }
  if (style.has_color()) {
    result <<= 1;
    result ^= std::hash<css::Color>()(style.color());
  }
  if (style.has_font_family()) {
    result <<= 1;
    result ^= std::hash<base::string16>()(style.font_family());
  }
  if (style.has_font_size()) {
    result <<= 1;
    result ^= std::hash<css::FontSize>()(style.font_size());
  }
  if (style.has_font_style()) {
    result <<= 1;
    result ^= std::hash<css::FontStyle>()(style.font_style());
  }
  if (style.has_font_weight()) {
    result <<= 1;
    result ^= std::hash<css::FontWeight>()(style.font_weight());
  }
  if (style.has_marker_color()) {
    result <<= 1;
    result ^= std::hash<css::Color>()(style.marker_color());
  }
  if (style.has_text_decoration()) {
    result <<= 1;
    result ^= std::hash<css::TextDecoration>()(style.text_decoration());
  }
  return result;
}
}  // namespace std
