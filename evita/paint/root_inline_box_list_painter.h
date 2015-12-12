// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_PAINT_ROOT_INLINE_BOX_LIST_PAINTER_H_
#define EVITA_PAINT_ROOT_INLINE_BOX_LIST_PAINTER_H_

#include <vector>

#include "base/macros.h"
#include "evita/gfx_base.h"

namespace views {
namespace rendering {
class RootInlineBox;
}
}

namespace paint {

class RootInlineBoxPainter;

//////////////////////////////////////////////////////////////////////
//
// RootInlineBoxListPainter
//
class RootInlineBoxListPainter final {
 public:
  using RootInlineBox = views::rendering::RootInlineBox;
  using FormatLineIterator = std::vector<RootInlineBox*>::const_iterator;

  RootInlineBoxListPainter(gfx::Canvas* canvas,
                           const gfx::RectF& bounds,
                           const gfx::ColorF& bgcolor,
                           const std::vector<RootInlineBox*>& format_lines,
                           const std::vector<RootInlineBox*>& screen_lines);
  ~RootInlineBoxListPainter();

  void Finish();
  bool Paint();
  FormatLineIterator TryCopy(const FormatLineIterator& format_line_start,
                             const FormatLineIterator& format_line_end) const;

 private:
  void Copy(float dst_top, float dst_bottom, float src_top) const;
  void DrawDirtyRect(const gfx::RectF& rect,
                     float red,
                     float green,
                     float blue) const;
  void FillBottom(const RootInlineBox* line) const;
  void FillRight(const RootInlineBox* line) const;
  FormatLineIterator FindFirstMismatch() const;
  FormatLineIterator FindLastMatch() const;
  std::vector<RootInlineBox*>::const_iterator FindCopyable(
      RootInlineBox* line) const;
  void RestoreSkipRect(const gfx::RectF& rect) const;

  const gfx::ColorF bgcolor_;
  const gfx::RectF& bounds_;
  gfx::Canvas* canvas_;
  mutable std::vector<gfx::RectF> copy_rects_;
  mutable std::vector<gfx::RectF> dirty_rects_;
  const std::vector<RootInlineBox*>& format_lines_;
  std::unique_ptr<RootInlineBoxPainter> root_box_painter_;
  const std::vector<RootInlineBox*>& screen_lines_;
  mutable std::vector<gfx::RectF> skip_rects_;

  DISALLOW_COPY_AND_ASSIGN(RootInlineBoxListPainter);
};

}  // namespace paint

#endif  // EVITA_PAINT_ROOT_INLINE_BOX_LIST_PAINTER_H_
