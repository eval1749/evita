// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/render_font_set.h"

#include <functional>
#include <unordered_map>

#include "base/logging.h"
#include "base/strings/string_util.h"
#include "common/memory/singleton.h"
#include "evita/css/style.h"
#include "evita/gfx/font_face.h"
#include "evita/views/text/render_font.h"

namespace {

gfx::FontProperties ComputeFontProperties(const base::string16& family_name,
                                          const css::Style& style) {
  gfx::FontProperties font_props;
  font_props.bold = css::FontWeight::Bold == style.font_weight();
  font_props.font_size_pt = style.font_size();
  font_props.italic = css::FontStyle::Italic == style.font_style();
  font_props.family_name = family_name;
  return font_props;
}

std::vector<base::string16> parseFontFamily(const base::string16& source) {
  enum class State{
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
        if (IsWhitespace(ch))
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
        if (!IsWhitespace(ch)) {
          name.push_back(' ');
          name.push_back(ch);
          state = State::Name;
        }
        break;
      case State::Start:
        if (ch != ',' && !IsWhitespace(ch)) {
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

namespace std {
template<> struct hash<views::rendering::FontSet::FontList> {
  size_t operator()(const views::rendering::FontSet::FontList& fonts) const {
    size_t result = 137u;
    for (auto const font : fonts) {
      result <<= 1;
      result ^= hash<const views::rendering::Font*>()(font);
    }
    return result;
  }
};
}  // namespace std

namespace views {
namespace rendering {

//////////////////////////////////////////////////////////////////////
//
// FontSet::Cache
//
class FontSet::Cache : public common::Singleton<FontSet::Cache> {
  DECLARE_SINGLETON_CLASS(FontSet::Cache);

  private: std::unordered_map<FontList, FontSet*> map_;

  private: Cache() = default;
  private: ~Cache() = default;

  public: const FontSet& GetOrCreate(const FontList& fonts);
};

const FontSet& FontSet::Cache::GetOrCreate(const FontList& fonts) {
  auto const present = map_.find(fonts);
  if (present != map_.end())
    return *present->second;
  auto new_font_set = new FontSet(fonts);
  map_[fonts] = new_font_set;
  return *new_font_set;
}

//////////////////////////////////////////////////////////////////////
//
// FontSet
//
FontSet::FontSet(const std::vector<const Font*>& fonts) : fonts_(fonts) {
}

FontSet::~FontSet() {
}

const Font* FontSet::FindFont(base::char16 sample) const {
  for (auto const font : fonts_) {
    if (font->HasCharacter(sample))
      return font;
  }
  return nullptr;
}

const FontSet& FontSet::Get(const css::Style& style) {
  FontList fonts;
  for (auto const font_family : parseFontFamily(style.font_family())) {
    const auto font_props = ComputeFontProperties(font_family, style);
    fonts.push_back(&Font::Get(font_props));
  }

  return FontSet::Cache::instance()->GetOrCreate(fonts);
}

const Font* FontSet::GetFont(const css::Style& style, base::char16 sample) {
  return Get(style).FindFont(sample);
}

}  // namespace rendering
}  // namespace views
