// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/layout/layout_view.h"

#include "evita/layout/layout_caret.h"
#include "evita/layout/root_inline_box.h"

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

}  // namespace views
