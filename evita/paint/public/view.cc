// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/paint/public/view.h"

#include "evita/paint/public/caret.h"
#include "evita/paint/public/line/root_inline_box.h"
#include "evita/paint/public/selection.h"

namespace paint {

View::View(int layout_version,
           const gfx::RectF& bounds,
           const std::vector<RootInlineBox*>& lines,
           scoped_refptr<Selection> selection,
           const gfx::ColorF& bgcolor,
           const gfx::RectF& ruler_bounds,
           std::unique_ptr<Caret> caret)
    : bgcolor_(bgcolor),
      bounds_(bounds),
      caret_(std::move(caret)),
      layout_version_(layout_version),
      lines_(lines),
      ruler_bounds_(ruler_bounds),
      selection_(selection) {}

View::~View() {
  for (const auto& line : lines_)
    line->Release();
}

}  // namespace paint
