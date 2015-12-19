// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_LINE_ROOT_INLINE_BOX_H_
#define EVITA_LAYOUT_LINE_ROOT_INLINE_BOX_H_

#include <stdint.h>

#include <vector>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "evita/gfx/rect_f.h"
#include "evita/text/offset.h"

namespace layout {

class InlineBox;
class Formatter;
class TextBlock;
class TextSelection;

class RootInlineBox final : public base::RefCounted<RootInlineBox> {
 public:
  RootInlineBox(const std::vector<InlineBox*>& boxes,
                text::Offset text_start,
                text::Offset text_end,
                float ascent,
                float descent);

  float bottom() const { return bounds_.bottom; }
  const gfx::PointF bottom_right() const { return bounds_.bottom_right(); }
  const gfx::RectF& bounds() const { return bounds_; }
  const std::vector<InlineBox*>& boxes() const { return boxes_; }
  float descent() const { return descent_; }
  float height() const { return bounds_.height(); }
  InlineBox* last_box() const { return boxes_.back(); }
  float left() const { return bounds_.left; }
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
  bool IsEndOfDocument() const;

 private:
  friend class base::RefCounted<RootInlineBox>;

  ~RootInlineBox();

  gfx::RectF bounds_;
  const std::vector<InlineBox*> boxes_;
  float descent_;
  text::Offset text_start_;
  text::Offset text_end_;

  DISALLOW_COPY_AND_ASSIGN(RootInlineBox);
};

}  // namespace layout

#endif  // EVITA_LAYOUT_LINE_ROOT_INLINE_BOX_H_
