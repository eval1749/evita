// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_GFX_FONT_H_
#define EVITA_GFX_FONT_H_

#include <functional>
#include <memory>
#include <string>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "evita/gfx/forward.h"

namespace gfx {
struct FontProperties;

//////////////////////////////////////////////////////////////////////
//
// Font
//
class Font {
 public:
  class Cache;
  friend class Cache;

  float descent() const { return metrics_.descent; }
  float height() const { return metrics_.height; }
  float underline() const { return metrics_.underline; }
  float underline_thickness() const { return metrics_.underline_thickness; }

  void DrawText(gfx::Canvas* canvas,
                const gfx::Brush& text_brush,
                const gfx::RectF& rect,
                const base::char16* chars,
                size_t num_chars) const;
  void DrawText(gfx::Canvas* canvas,
                const gfx::Brush& text_brush,
                const gfx::RectF& rect,
                const base::string16& string) const;
  static const Font& Get(const gfx::FontProperties& properties);
  float GetCharWidth(base::char16 ch) const;
  float GetTextWidth(const base::char16* chars, size_t num_chars) const;
  float GetTextWidth(const base::string16& string) const;
  bool HasCharacter(base::char16) const;

 private:
  class FontImpl;

  struct SimpleMetrics {
    float ascent;
    float descent;
    float height;
    float fixed_width;
    float underline;
    float underline_thickness;
  };

  explicit Font(const gfx::FontProperties& properties);
  ~Font();

  float ascent() const { return metrics_.ascent; }

  const std::unique_ptr<FontImpl> font_impl_;
  const SimpleMetrics metrics_;

  DISALLOW_COPY_AND_ASSIGN(Font);
};

}  // namespace gfx

namespace std {
template <>
struct hash<gfx::Font> {
  size_t operator()(const gfx::Font& font) const;
};
}  // namespace std

#endif  // EVITA_GFX_FONT_H_
