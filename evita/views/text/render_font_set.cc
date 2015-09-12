// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/render_font_set.h"

#include <functional>
#include <unordered_map>

#include "base/logging.h"
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

}  // namespace

namespace std {
template <>
struct hash<views::rendering::FontSet::FontList> {
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

 public:
  const FontSet& GetOrCreate(const FontList& fonts);

 private:
  Cache() = default;
  ~Cache() = default;

  std::unordered_map<FontList, FontSet*> map_;

  DISALLOW_COPY_AND_ASSIGN(Cache);
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
FontSet::FontSet(const std::vector<const Font*>& fonts) : fonts_(fonts) {}

FontSet::~FontSet() {}

const Font* FontSet::FindFont(base::char16 sample) const {
  for (auto const font : fonts_) {
    if (font->HasCharacter(sample))
      return font;
  }
  return nullptr;
}

const FontSet& FontSet::Get(const css::Style& style) {
  FontList fonts;
  for (auto const font_family : style.font_families()) {
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
