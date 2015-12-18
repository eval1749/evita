// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_TEXT_VIEW_H_
#define EVITA_LAYOUT_TEXT_VIEW_H_

#include <memory>

#include "base/memory/ref_counted.h"
#include "evita/gfx/canvas.h"
#include "evita/gfx/canvas_observer.h"
#include "evita/gfx/rect.h"
#include "evita/gfx/rect_f.h"
#include "evita/layout/render_selection.h"

namespace base {
class Time;
}

namespace paint {
class ViewPaintCache;
class View;
}

namespace text {
class Buffer;
class Offset;
}

namespace ui {
class AnimatableWindow;
}

namespace layout {

class LayoutBlockFlow;
class PaintViewBuilder;

//////////////////////////////////////////////////////////////////////
//
// TextView
//
class TextView final : private gfx::CanvasObserver {
 public:
  TextView(text::Buffer* buffer, ui::AnimatableWindow* caret_owner);
  ~TextView();

  text::Buffer* buffer() const { return buffer_; }

  void DidChangeStyle(text::Offset offset, text::OffsetDelta length);
  void DidDeleteAt(text::Offset offset, text::OffsetDelta length);
  void DidHide();
  void DidInsertBefore(text::Offset offset, text::OffsetDelta length);
  // Returns end of line offset containing |text_offset|.
  text::Offset EndOfLine(text::Offset text_offset) const;
  text::Offset GetStart();
  text::Offset GetEnd();
  // Returns fully visible end offset or end of line position if there is only
  // one line.
  text::Offset GetVisibleEnd();
  void Format(text::Offset text_offset);
  // Returns true if text format is taken place.
  bool FormatIfNeeded();
  gfx::RectF HitTestTextPosition(text::Offset text_offset) const;
  void MakeSelectionVisible();
  text::Offset HitTestPoint(gfx::PointF point);
  text::Offset MapPointXToOffset(text::Offset text_offset, float point_x) const;
  void Paint(gfx::Canvas* canvas);
  bool ScrollDown();
  bool ScrollUp();
  void SetBounds(const gfx::RectF& new_bounds);
  void SetZoom(float new_zoom);
  // Returns start of line offset containing |text_offset|.
  text::Offset StartOfLine(text::Offset text_offset) const;
  void Update(const TextSelectionModel& selection, base::Time now);

 private:
  bool IsPositionFullyVisible(text::Offset text_offset) const;
  void ScrollToPosition(text::Offset offset);

  // gfx::CanvasObserver
  void DidRecreateCanvas() final;

  gfx::RectF bounds_;
  text::Buffer* const buffer_;
  text::Offset caret_offset_;
  std::unique_ptr<LayoutBlockFlow> layout_block_flow_;
  scoped_refptr<paint::View> paint_view_;
  const std::unique_ptr<PaintViewBuilder> paint_view_builder_;
  std::unique_ptr<paint::ViewPaintCache> view_paint_cache_;

  DISALLOW_COPY_AND_ASSIGN(TextView);
};

}  // namespace layout

#endif  // EVITA_LAYOUT_TEXT_VIEW_H_
