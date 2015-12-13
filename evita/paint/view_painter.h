// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_PAINT_VIEW_PAINTER_H_
#define EVITA_PAINT_VIEW_PAINTER_H_

#include <memory>

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

class ViewPaintCache;

//////////////////////////////////////////////////////////////////////
//
// ViewPainter
//
class ViewPainter final {
 public:
  using LayoutView = views::LayoutView;

  ViewPainter(gfx::Canvas* canvas,
              base::Time now,
              std::unique_ptr<ViewPaintCache> view_cache);
  ~ViewPainter();

  std::unique_ptr<ViewPaintCache> Paint(const LayoutView& layout_view);

 private:
  void PaintRuler(const LayoutView& layout_view);
  void PaintSelection(const LayoutView& layout_view);
  void PaintSelectionIfNeeded(const LayoutView& layout_view);
  void UpdateCaret(const LayoutView& layout_view);

  gfx::Canvas* const canvas_;
  const base::Time now_;
  std::unique_ptr<ViewPaintCache> view_cache_;

  DISALLOW_COPY_AND_ASSIGN(ViewPainter);
};

}  // namespace paint

#endif  // EVITA_PAINT_VIEW_PAINTER_H_
