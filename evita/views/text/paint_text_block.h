// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TEXT_PAINT_TEXT_BLOCK_H_
#define EVITA_VIEWS_TEXT_PAINT_TEXT_BLOCK_H_

#include <vector>

#include "base/macros.h"
#include "evita/gfx_base.h"
#include "evita/views/text/render_selection.h"

namespace views {

namespace rendering {
class TextLine;
};

class PaintTextBlock final {
 public:
  using TextLine = rendering::TextLine;
  using TextSelection = rendering::TextSelection;

  PaintTextBlock(const std::vector<TextLine*>& lines,
                 const TextSelection& selection,
                 const gfx::ColorF& bgcolor);
  ~PaintTextBlock();

  const gfx::ColorF& bgcolor() const { return bgcolor_; }
  const std::vector<TextLine*>& lines() const { return lines_; }
  const TextSelection& selection() const { return selection_; }

 private:
  const gfx::ColorF bgcolor_;
  const std::vector<TextLine*> lines_;
  const TextSelection selection_;

  DISALLOW_COPY_AND_ASSIGN(PaintTextBlock);
};

}  // namespace views

#endif  // EVITA_VIEWS_TEXT_PAINT_TEXT_BLOCK_H_
