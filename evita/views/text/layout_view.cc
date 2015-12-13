// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/layout_view.h"

#include "evita/views/text/layout_caret.h"
#include "evita/views/text/root_inline_box.h"

namespace views {

LayoutView::LayoutView(int layout_version,
                       const gfx::RectF& bounds,
                       const std::vector<RootInlineBox*>& lines,
                       const TextSelection& selection,
                       const gfx::ColorF& bgcolor,
                       const gfx::RectF& ruler_bounds,
                       std::unique_ptr<LayoutCaret> caret)
    : bgcolor_(bgcolor),
      bounds_(bounds),
      caret_(std::move(caret)),
      layout_version_(layout_version),
      lines_(lines),
      ruler_bounds_(ruler_bounds),
      selection_(selection) {}

LayoutView::~LayoutView() {
  for (const auto& line : lines_)
    delete line;
}

gfx::RectF LayoutView::HitTestTextPosition(text::Posn offset) const {
  if (offset < lines_.front()->text_start() ||
      offset > lines_.back()->text_end()) {
    return gfx::RectF();
  }
  for (auto const line : lines_) {
    auto const rect = line->HitTestTextPosition(offset);
    if (!rect.empty())
      return rect;
  }
  return gfx::RectF();
}

}  // namespace views
