// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_render_font_set_h)
#define INCLUDE_evita_views_text_render_font_set_h

#include <vector>

#include "base/strings/string16.h"

namespace css {
class Style;
}

namespace views {
namespace rendering {

class Font;

//////////////////////////////////////////////////////////////////////
//
// FontSet
//
class FontSet {
  private: class Cache;
  friend class Cache;

  public: typedef std::vector<const Font*> FontList;

  private: FontList fonts_;

  private: FontSet(const std::vector<const Font*>& fonts);
  private: ~FontSet();

  private: const Font* FindFont(base::char16 sample) const;
  private: static const FontSet& Get(const css::Style& style);
  public: static const Font* GetFont(const css::Style& style,
                                     base::char16 sample);

  DISALLOW_COPY_AND_ASSIGN(FontSet);
};

}  // namespace rendering
}  // namespace views

#endif //!defined(INCLUDE_evita_views_text_render_font_set_h)
