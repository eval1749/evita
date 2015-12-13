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

  ViewPainter(const LayoutView& layout_view);
  ~ViewPainter();

  std::unique_ptr<ViewPaintCache> Paint(
      gfx::Canvas* canvas,
      base::Time now,
      std::unique_ptr<ViewPaintCache> view_cache);

 private:
  ui::Caret* caret() const;

  void PaintRuler(gfx::Canvas* canvas);
  void PaintSelection(gfx::Canvas* canvas, base::Time now);
  void PaintSelectionIfNeeded(gfx::Canvas* canvas,
                              base::Time now,
                              const ViewPaintCache& view_cache);
  void UpdateCaret(gfx::Canvas* canvas, base::Time now);

  const LayoutView& layout_view_;

  DISALLOW_COPY_AND_ASSIGN(ViewPainter);
};

}  // namespace paint

#endif  // EVITA_PAINT_VIEW_PAINTER_H_
