// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_PAINT_PUBLIC_LINE_ROOT_INLINE_BOX_H_
#define EVITA_PAINT_PUBLIC_LINE_ROOT_INLINE_BOX_H_

#include <stdint.h>

#include <vector>

#include "base/memory/ref_counted.h"
#include "evita/gfx/rect_f.h"

namespace layout {
// TODO(eval1749) We should have paint::InlineBox.
class InlineBox;
}

namespace paint {

class RootInlineBox final : public base::RefCounted<RootInlineBox> {
  // TODO(eval1749): We should use |paint::InlineBox| instead of
  // |layout::InlineBox|.
  using InlineBox = layout::InlineBox;

 public:
  RootInlineBox(const std::vector<InlineBox*>& boxes, const gfx::RectF& bounds);
  RootInlineBox(const RootInlineBox& other);

  float bottom() const { return bounds_.bottom; }
  const gfx::PointF bottom_right() const { return bounds_.bottom_right(); }
  const gfx::RectF& bounds() const { return bounds_; }
  const std::vector<InlineBox*>& boxes() const { return boxes_; }
  float height() const { return bounds_.height(); }
  InlineBox* last_box() const { return boxes_.back(); }
  float left() const { return bounds_.left; }
  float right() const { return bounds_.right; }
  float top() const { return bounds_.top; }
  const gfx::PointF origin() const { return bounds_.origin(); }
  float width() const { return bounds_.width(); }

  RootInlineBox* Copy() const;
  bool Equal(const RootInlineBox*) const;

 private:
  friend class base::RefCounted<RootInlineBox>;

  ~RootInlineBox();

  size_t ComputeHashCode() const;

  const gfx::RectF bounds_;
  const std::vector<InlineBox*> boxes_;
  mutable size_t hash_code_;
};

}  // namespace paint

#endif  // EVITA_PAINT_PUBLIC_LINE_ROOT_INLINE_BOX_H_
