// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_PAINT_VIEW_PAINT_CACHE_H_
#define EVITA_PAINT_VIEW_PAINT_CACHE_H_

#include <vector>

#include "base/macros.h"
#include "evita/gfx/forward.h"
#include "evita/views/text/render_selection.h"

namespace views {
class LayoutView;
namespace rendering {
class RootInlineBox;
}
}

namespace paint {

//////////////////////////////////////////////////////////////////////
//
// ViewPaintCache
//
class ViewPaintCache final {
  using LayoutView = views::LayoutView;
  using RootInlineBox = views::rendering::RootInlineBox;
  using TextSelection = views::rendering::TextSelection;

 public:
  ViewPaintCache(gfx::Canvas* canvas, const LayoutView& view);
  ~ViewPaintCache();

  const std::vector<RootInlineBox*>& lines() const { return lines_; }
  const TextSelection& selection() const { return selection_; }

  bool CanUseTextImage(gfx::Canvas* canvas) const;
  bool NeedsTextPaint(gfx::Canvas* canvas, const LayoutView& view) const;
  void UpdateSelection(const TextSelection& selection);

 private:
  const int bitmap_id_;
  const gfx::Canvas* const canvas_;
  const int layout_version_;
  std::vector<RootInlineBox*> lines_;
  TextSelection selection_;

  DISALLOW_COPY_AND_ASSIGN(ViewPaintCache);
};

}  // namespace paint

#endif  // EVITA_PAINT_VIEW_PAINT_CACHE_H_
