// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/paint_text_block.h"

#include "evita/views/text/render_text_line.h"

namespace views {

PaintTextBlock::PaintTextBlock(const std::vector<TextLine*>& lines,
                               const TextSelection& selection,
                               const gfx::ColorF& bgcolor)
    : bgcolor_(bgcolor), lines_(lines), selection_(selection) {}

PaintTextBlock::~PaintTextBlock() {
  for (const auto& line : lines_)
    delete line;
}

}  // namespace views
