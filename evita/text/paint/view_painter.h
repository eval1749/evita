// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_PAINT_VIEW_PAINTER_H_
#define EVITA_TEXT_PAINT_VIEW_PAINTER_H_

#include <memory>

#include "base/macros.h"
#include "evita/gfx/rect_f.h"

namespace gfx {
class Canvas;
}

namespace paint {

class View;
class ViewPaintCache;

//////////////////////////////////////////////////////////////////////
//
// ViewPainter
//
class ViewPainter final {
 public:
  explicit ViewPainter(const View& layout_view);
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

  const View& layout_view_;
  gfx::RectF caret_bounds_;

  DISALLOW_COPY_AND_ASSIGN(ViewPainter);
};

}  // namespace paint

#endif  // EVITA_TEXT_PAINT_VIEW_PAINTER_H_
