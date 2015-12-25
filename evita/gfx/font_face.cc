// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/font_face.h"

#include "evita/gfx/direct_write_factory_win.h"

namespace gfx {

namespace {

common::ComPtr<IDWriteFontFace> CreateFontFace(
    const FontProperties& properties) {
  common::ComPtr<IDWriteFontCollection> font_collection;
  COM_VERIFY(DirectWriteFactory::GetInstance()->impl()->GetSystemFontCollection(
      &font_collection, false));

  uint32 index;
  BOOL exists;
  COM_VERIFY(font_collection->FindFamilyName(properties.family_name.c_str(),
                                             &index, &exists));
  if (!exists) {
    FontProperties fallback_properties;
    fallback_properties.family_name = L"Courier New";
    return CreateFontFace(fallback_properties);
  }

  common::ComPtr<IDWriteFontFamily> font_family;
  COM_VERIFY(font_collection->GetFontFamily(index, &font_family));

  common::ComPtr<IDWriteFont> font;
  COM_VERIFY(font_family->GetFirstMatchingFont(
      properties.bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL,
      properties.italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
      &font));

  common::ComPtr<IDWriteFontFace> font_face;
  COM_VERIFY(font->CreateFontFace(&font_face));
  return std::move(font_face);
}

DWRITE_FONT_METRICS GetFontMetrics(IDWriteFontFace* font) {
  DWRITE_FONT_METRICS metrics;
  font->GetMetrics(&metrics);
  return metrics;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Font::Properties
//
FontProperties::FontProperties()
    : bold(false), italic(false), font_size_pt(0) {}

bool FontProperties::operator==(const FontProperties& other) const {
  return bold == other.bold && font_size_pt == other.font_size_pt &&
         family_name == other.family_name && italic == other.italic;
}

bool FontProperties::operator!=(const FontProperties& other) const {
  return !operator==(other);
}

//////////////////////////////////////////////////////////////////////
//
// FontSet
//
FontFace::FontFace(const FontProperties& properties)
    : SimpleObject_(CreateFontFace(properties)),
      metrics_(GetFontMetrics(*this)) {}

FontFace::~FontFace() {}

}  // namespace gfx

namespace std {
size_t hash<gfx::FontProperties>::operator()(
    const gfx::FontProperties& properties) const {
  size_t result = 137u;
  result <<= 1;
  result ^= hash<bool>()(properties.bold);
  result <<= 1;
  result ^= hash<bool>()(properties.italic);
  result <<= 1;
  result ^= hash<float>()(properties.font_size_pt);
  result <<= 1;
  result ^= hash<base::string16>()(properties.family_name);
  return result;
}

}  // namespace std
