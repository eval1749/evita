// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/paint/view_paint_cache.h"

#include "evita/gfx/canvas.h"
#include "evita/views/text/layout_caret.h"
#include "evita/views/text/layout_view.h"
#include "evita/views/text/root_inline_box.h"

namespace paint {

//////////////////////////////////////////////////////////////////////
//
// ViewPaintCache
//
ViewPaintCache::ViewPaintCache(gfx::Canvas* canvas,
                               const LayoutView& view,
                               const gfx::RectF& caret_bounds)
    : bitmap_id_(canvas->bitmap_id()),
      caret_bounds_(caret_bounds),
      canvas_(canvas),
      layout_version_(view.layout_version()),
      selection_(view.selection()) {
  for (const auto& line : view.lines())
    lines_.push_back(line->Copy());
}

ViewPaintCache::~ViewPaintCache() {
  for (const auto& line : lines_)
    delete line;
}

bool ViewPaintCache::CanUseTextImage(gfx::Canvas* canvas) const {
  return canvas_ == canvas && bitmap_id_ == canvas->bitmap_id();
}

bool ViewPaintCache::NeedsTextPaint(gfx::Canvas* canvas,
                                    const LayoutView& view) const {
  return CanUseTextImage(canvas) && layout_version_ != view.layout_version();
}

void ViewPaintCache::UpdateSelection(const TextSelection& selection,
                                     const gfx::RectF& caret_bounds) {
  caret_bounds_ = caret_bounds;
  selection_ = selection;
}

}  // namespace pain
