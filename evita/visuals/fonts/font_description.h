// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_FONTS_FONT_DESCRIPTION_H_
#define EVITA_VISUALS_FONTS_FONT_DESCRIPTION_H_

#include "base/strings/string16.h"

namespace visuals {

// Note: Order of enum member is matched to |DWRITE_FONT_STRETCH|.
enum class FontStretch {
  UltraCondensed = 1,
  ExtraCondensed = 2,
  Condensed = 3,
  SemiCondensed = 4,
  Normal = 5,
  SemiExpanded = 6,
  Expanded = 7,
  ExtraExpanded = 8,
  UltraExpanded = 9,
};

// Note: Order of enum members is matched to |DWRITE_FONT_STYLE|.
enum class FontStyle {
  Normal,
  Oblique,
  Italic,
};

enum class FontWeight {
  k100 = 100,
  k200 = 200,
  k300 = 300,
  k400 = 400,
  k500 = 500,
  k600 = 600,
  k700 = 700,
  k800 = 800,
  k900 = 900,
  k350 = 350,  // Direct Write SemiLight
  k950 = 950,  // Direct Write Ultra Black
  Bold = k700,
  Normal = k400,
};

//////////////////////////////////////////////////////////////////////
//
// FontDescription
//
class FontDescription final {
 public:
  class Builder;

  FontDescription(const FontDescription& other);
  FontDescription();
  ~FontDescription();

  FontDescription& operator=(const FontDescription& other);

  bool operator==(const FontDescription& other) const;
  bool operator!=(const FontDescription& other) const;

  const base::string16& family() const { return family_; }
  float size() const { return size_; }
  FontStretch stretch() const { return stretch_; }
  FontStyle style() const { return style_; }
  FontWeight weight() const { return weight_; }

 private:
  base::string16 family_;
  float size_ = 0;
  FontStretch stretch_ = FontStretch::Normal;
  FontStyle style_ = FontStyle::Normal;
  FontWeight weight_ = FontWeight::Normal;
};

}  // namespace visuals

#endif  // EVITA_VISUALS_FONTS_FONT_DESCRIPTION_H_
