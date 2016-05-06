// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_STYLE_COMPUTED_STYLE_BUILDER_H_
#define EVITA_TEXT_STYLE_COMPUTED_STYLE_BUILDER_H_

#include "base/macros.h"
#include "evita/text/style/computed_style.h"

namespace layout {

//////////////////////////////////////////////////////////////////////
//
// ComputedStyle::Builder
//
class ComputedStyle::Builder {
 public:
  Builder();
  ~Builder();

  ComputedStyle Build();

  Builder& Load(const css::Style& model, const gfx::Font& font);
  Builder& SetBackgroundColor(const gfx::ColorF& color);
  Builder& SetColor(const gfx::ColorF& color);
  Builder& SetFont(const gfx::Font& font);
  Builder& SetTextDecoration(css::TextDecoration decoration);

 private:
  ComputedStyle style_;

  DISALLOW_COPY_AND_ASSIGN(Builder);
};

}  // namespace layout

#endif  // EVITA_TEXT_STYLE_COMPUTED_STYLE_BUILDER_H_
