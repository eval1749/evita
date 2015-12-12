// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_PAINT_LAYOUT_VIEW_PAINTER_H_
#define EVITA_PAINT_LAYOUT_VIEW_PAINTER_H_

#include "base/macros.h"
#include "evita/gfx/forward.h"

namespace views {
class LayoutView;
}

namespace paint {

//////////////////////////////////////////////////////////////////////
//
// LayoutViewPainter
//
class LayoutViewPainter final {
 public:
  LayoutViewPainter(gfx::Canvas* canvas);
  ~LayoutViewPainter();

  void Paint(const views::LayoutView& layout_view);

 private:
  gfx::Canvas* const canvas_;

  DISALLOW_COPY_AND_ASSIGN(LayoutViewPainter);
};

}  // namespace paint

#endif  // EVITA_PAINT_LAYOUT_VIEW_PAINTER_H_
