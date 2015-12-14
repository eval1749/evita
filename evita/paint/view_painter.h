// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_PAINT_VIEW_PAINTER_H_
#define EVITA_PAINT_VIEW_PAINTER_H_

#include <memory>

#include "base/macros.h"
#include "evita/gfx/rect_f.h"

namespace gfx {
class Canvas;
}

namespace layout {
class LayoutView;
}

namespace paint {

class ViewPaintCache;

//////////////////////////////////////////////////////////////////////
//
// ViewPainter
//
class ViewPainter final {
  using LayoutView = layout::LayoutView;

 public:
  ViewPainter(const LayoutView& layout_view);
  ~ViewPainter();

  std::unique_ptr<ViewPaintCache> Paint(
      gfx::Canvas* canvas,
      std::unique_ptr<ViewPaintCache> view_cache);

 private:
  void PaintCaretIfNeeded(gfx::Canvas* canvas);
  void PaintRuler(gfx::Canvas* canvas);
  void PaintSelection(gfx::Canvas* canvas);
  void PaintSelectionWithCache(gfx::Canvas* canvas,
                               const ViewPaintCache& view_cache);
  void RestoreCaretBackgroundIfNeeded(gfx::Canvas* canvas,
                                      const ViewPaintCache& view_cache);

  const LayoutView& layout_view_;
  gfx::RectF caret_bounds_;

  DISALLOW_COPY_AND_ASSIGN(ViewPainter);
};

}  // namespace paint

#endif  // EVITA_PAINT_VIEW_PAINTER_H_
