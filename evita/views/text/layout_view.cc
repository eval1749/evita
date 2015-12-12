// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/views/text/layout_view.h"

#include "evita/views/text/root_inline_box.h"

namespace views {

LayoutView::LayoutView(int layout_version,
                       const std::vector<RootInlineBox*>& lines,
                       const TextSelection& selection,
                       const gfx::ColorF& bgcolor)
    : bgcolor_(bgcolor),
      layout_version_(layout_version),
      lines_(lines),
      selection_(selection) {}

LayoutView::~LayoutView() {
  for (const auto& line : lines_)
    delete line;
}

}  // namespace views
