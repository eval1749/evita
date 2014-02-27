// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_render_style_h)
#define INCLUDE_evita_views_text_render_style_h

#include <functional>

#include "base/basictypes.h"
#include "evita/gfx_base.h"
#include "evita/css/style.h"
#include "evita/vi_Style.h"

namespace views {
namespace rendering {

class RenderStyle {
  private: gfx::ColorF bgcolor_;
  private: gfx::ColorF color_;
  private: Font* font_;
  private: text::TextDecoration text_decoration_;

  public: RenderStyle(const text::Style& values, Font* font);
  public: RenderStyle(const RenderStyle& other);
  public: ~RenderStyle();

  public: bool operator==(const RenderStyle& other) const;
  public: bool operator!=(const RenderStyle& other) const;

  public: gfx::ColorF bgcolor() const { return bgcolor_; }
  public: gfx::ColorF color() const { return color_; }
  public: Font* font() const { return font_; }
  public: text::TextDecoration text_decoration() const {
    return text_decoration_;
  }
};

} // namespace rendering
} // namespace views

namespace std {
template<> struct hash<views::rendering::RenderStyle> {
  size_t operator()(const views::rendering::RenderStyle& style) const;
};
}  // namespace std

#endif //!defined(INCLUDE_evita_views_text_render_style_h)
