// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_TEXT_VIEW_H_
#define EVITA_LAYOUT_TEXT_VIEW_H_

#include <memory>

#include "base/memory/ref_counted.h"
#include "evita/gfx/rect.h"
#include "evita/gfx/rect_f.h"
#include "evita/layout/render_selection.h"

namespace base {
class TimeTicks;
}

namespace paint {
class Caret;
class View;
}

namespace text {
class Buffer;
class Offset;
class StaticRange;
}

namespace ui {
class AnimatableWindow;
}

namespace layout {

class BlockFlow;

//////////////////////////////////////////////////////////////////////
//
// TextView
//
class TextView final {
  using CaretDisplayItem = paint::Caret;

 public:
  explicit TextView(const text::Buffer& buffer);
  ~TextView();

  const BlockFlow& block() const { return *block_; }
  const text::Buffer& buffer() const { return buffer_; }
  text::Offset text_end() const;
  text::Offset text_start() const;
  float zoom() const;

  // Returns end of line offset containing |text_offset|.
  text::Offset ComputeEndOfLine(text::Offset text_offset) const;
  gfx::RectF ComputeCaretBounds(const TextSelectionModel& selection) const;
  // Returns start of line offset containing |text_offset|.
  text::Offset ComputeStartOfLine(text::Offset text_offset) const;
  // Returns fully visible end offset or end of line position if there is only
  // one line.
  text::Offset ComputeVisibleEnd() const;
  void DidChangeStyle(const text::StaticRange& range);
  void DidDeleteAt(const text::StaticRange& range);
  void DidInsertBefore(const text::StaticRange& range);
  void Format(text::Offset text_offset);
  // Returns true if text format is taken place.
  bool FormatIfNeeded();
  gfx::RectF HitTestTextPosition(text::Offset text_offset) const;
  void MakeSelectionVisible();
  text::Offset HitTestPoint(gfx::PointF point);
  text::Offset MapPointXToOffset(text::Offset text_offset, float point_x) const;
  bool ScrollDown();
  bool ScrollUp();
  void SetBounds(const gfx::RectF& new_bounds);
  void SetZoom(float new_zoom);
  void Update(const TextSelectionModel& selection);

 private:
  void ScrollToPosition(text::Offset offset);

  std::unique_ptr<BlockFlow> block_;
  const text::Buffer& buffer_;
  text::Offset caret_offset_;

  DISALLOW_COPY_AND_ASSIGN(TextView);
};

}  // namespace layout

#endif  // EVITA_LAYOUT_TEXT_VIEW_H_
