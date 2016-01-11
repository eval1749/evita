// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VISUALS_FONTS_FONT_DESCRIPTION_H_
#define EVITA_VISUALS_FONTS_FONT_DESCRIPTION_H_

#include "base/strings/string16.h"

namespace visuals {

enum class FontStretch {
  UltraCondensed,
  ExtraCondensed,
  Condensed,
  SemiCondensed,
  Normal,
  Medium,
  SemiExpanded,
  Expanded,
  ExtraExpanded,
  UltraExpanded,
};

enum class FontStyle {
  Normal,
  Italic,
  Oblique,
};

enum class FontWeight {
  k100,
  k200,
  k300,
  k400,
  k500,
  k600,
  k700,
  k800,
  k900,
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
