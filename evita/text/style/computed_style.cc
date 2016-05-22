// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/style/computed_style.h"

#include "evita/gfx/font.h"

namespace layout {

ComputedStyle::ComputedStyle(ComputedStyle&& other)
    : bgcolor_(other.bgcolor_),
      color_(other.color_),
      fonts_(std::move(other.fonts_)),
      text_decoration_color_(other.text_decoration_color_),
      text_decoration_line_(other.text_decoration_line_),
      text_decoration_style_(other.text_decoration_style_) {}

ComputedStyle::ComputedStyle() = default;
ComputedStyle::~ComputedStyle() = default;

}  // namespace layout
