// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_STYLE_COMPUTED_STYLE_BUILDER_H_
#define EVITA_TEXT_STYLE_COMPUTED_STYLE_BUILDER_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "evita/gfx/color_f.h"
#include "evita/text/style/computed_style.h"

namespace gfx {
class Font;
}

namespace layout {

//////////////////////////////////////////////////////////////////////
//
// ComputedStyle::Builder
//
class ComputedStyle::Builder {
 public:
  Builder();
  ~Builder();

  const gfx::ColorF& background_color() const { return style_->bgcolor(); }
  const gfx::ColorF& color() const { return style_->color(); }

  std::unique_ptr<ComputedStyle> Build();

  Builder& SetBackgroundColor(const gfx::ColorF& color);
  Builder& SetColor(const gfx::ColorF& color);
  Builder& SetFonts(const std::vector<const gfx::Font*>& fonts);
  Builder& SetTextDecorationColor(const gfx::ColorF& color);
  Builder& SetTextDecorationLine(TextDecorationLine line);
  Builder& SetTextDecorationStyle(TextDecorationStyle style);

 private:
  std::unique_ptr<ComputedStyle> style_;

  DISALLOW_COPY_AND_ASSIGN(Builder);
};

}  // namespace layout

#endif  // EVITA_TEXT_STYLE_COMPUTED_STYLE_BUILDER_H_
