// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_RENDER_FONT_SET_H_
#define EVITA_LAYOUT_RENDER_FONT_SET_H_

#include <vector>

#include "base/strings/string16.h"

namespace css {
class Style;
}

namespace gfx {
class Font;
}

namespace layout {

//////////////////////////////////////////////////////////////////////
//
// FontSet
//
class FontSet {
 public:
  using FontList = std::vector<const gfx::Font*>;

  static const gfx::Font* GetFont(const css::Style& style, base::char16 sample);

 private:
  class Cache;
  friend class Cache;

  explicit FontSet(const FontList& fonts);
  ~FontSet();

  const gfx::Font* FindFont(base::char16 sample) const;
  static const FontSet& Get(const css::Style& style);

  FontList fonts_;

  DISALLOW_COPY_AND_ASSIGN(FontSet);
};

}  // namespace layout

#endif  // EVITA_LAYOUT_RENDER_FONT_SET_H_
