// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_LINE_ROOT_INLINE_BOX_H_
#define EVITA_LAYOUT_LINE_ROOT_INLINE_BOX_H_

#include <stdint.h>

#include <vector>

#include "base/macros.h"
#include "evita/gfx/rect_f.h"
#include "evita/text/offset.h"

namespace layout {

class InlineBox;
class Formatter;
class TextBlock;
class TextSelection;

class RootInlineBox final {
 public:
  RootInlineBox(const std::vector<InlineBox*>& boxes,
                text::Offset line_start,
                text::Offset text_start,
                text::Offset text_end,
                float ascent,
                float descent);
  ~RootInlineBox();

  float bottom() const { return bounds_.bottom; }
  const gfx::PointF bottom_right() const { return bounds_.bottom_right(); }
  const gfx::RectF& bounds() const { return bounds_; }
  const std::vector<InlineBox*>& boxes() const { return boxes_; }
  float descent() const { return descent_; }
  float height() const { return bounds_.height(); }
  InlineBox* last_box() const { return boxes_.back(); }
  float left() const { return bounds_.left; }
  text::Offset line_start() const { return line_start_; }
  float right() const { return bounds_.right; }
  float top() const { return bounds_.top; }
  const gfx::PointF origin() const { return bounds_.origin(); }
  void set_origin(const gfx::PointF& origin);
  text::Offset text_end() const { return text_end_; }
  text::Offset text_start() const { return text_start_; }
  float width() const { return bounds_.width(); }

  bool Contains(text::Offset offset) const;
  text::Offset HitTestPoint(float x) const;
  gfx::RectF HitTestTextPosition(text::Offset offset) const;
  bool IsContinuedLine() const { return line_start_ != text_start_; }
  bool IsEndOfDocument() const;
  void UpdateTextStart(text::OffsetDelta delta);

 private:
  gfx::RectF bounds_;
  const std::vector<InlineBox*> boxes_;
  const float descent_;
  text::Offset line_start_;
  text::Offset text_start_;
  text::Offset text_end_;

  DISALLOW_COPY_AND_ASSIGN(RootInlineBox);
};

}  // namespace layout

#endif  // EVITA_LAYOUT_LINE_ROOT_INLINE_BOX_H_
