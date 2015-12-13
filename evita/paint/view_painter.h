// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_PAINT_VIEW_PAINTER_H_
#define EVITA_PAINT_VIEW_PAINTER_H_

#include "base/macros.h"
#include "base/time/time.h"
#include "evita/gfx/forward.h"

namespace ui {
class Caret;
}

namespace views {
class LayoutView;
}

namespace paint {

//////////////////////////////////////////////////////////////////////
//
// ViewPainter
//
class ViewPainter final {
 public:
  using LayoutView = views::LayoutView;

  ViewPainter(gfx::Canvas* canvas,
              base::Time now,
              const LayoutView* last_layout_view);
  ~ViewPainter();

  void Paint(const LayoutView& layout_view);

 private:
  void PaintRuler(const LayoutView& layout_view);
  void PaintSelection(const LayoutView& layout_view);
  void PaintSelectionIfNeeded(const LayoutView& layout_view);
  void UpdateCaret(const LayoutView& layout_view);

  gfx::Canvas* const canvas_;
  const LayoutView* last_layout_view_;
  const base::Time now_;

  DISALLOW_COPY_AND_ASSIGN(ViewPainter);
};

}  // namespace paint

#endif  // EVITA_PAINT_VIEW_PAINTER_H_
