// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_LINE_ROOT_INLINE_BOX_H_
#define EVITA_LAYOUT_LINE_ROOT_INLINE_BOX_H_

#include <stdint.h>

#include <vector>

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
  RootInlineBox();

  float bottom() const { return bounds_.bottom; }
  const gfx::PointF& bottom_right() const { return bounds_.bottom_right(); }
  const gfx::RectF& bounds() const { return bounds_; }
  const std::vector<InlineBox*>& cells() const { return cells_; }
  float height() const { return bounds_.height(); }
  InlineBox* last_cell() const { return cells_.back(); }
  float left() const { return bounds_.left; }
  float right() const { return bounds_.right; }
  float top() const { return bounds_.top; }
  const gfx::PointF origin() const { return bounds_.origin(); }
  void set_origin(const gfx::PointF& origin);
  void set_start(text::Offset start) { m_lStart = start; }
  text::Offset text_end() const { return m_lEnd; }
  text::Offset text_start() const { return m_lStart; }
  float width() const { return bounds_.width(); }

  void AddInlineBox(InlineBox* cell);
  gfx::RectF CalculateSelectionRect(const TextSelection& selection) const;
  RootInlineBox* Copy() const;
  bool Equal(const RootInlineBox*) const;
  void Fix(float ascent, float descent);
  uint32_t Hash() const;
  gfx::RectF HitTestTextPosition(text::Offset lPosn) const;
  bool IsEndOfDocument() const;
  text::Offset MapXToPosn(float x) const;

 private:
  friend class base::RefCounted<RootInlineBox>;

  RootInlineBox(const RootInlineBox& other);
  ~RootInlineBox();

  bool Contains(text::Offset offset) const;

  gfx::RectF bounds_;
  std::vector<InlineBox*> cells_;
  mutable uint32_t m_nHash;
  text::Offset m_lStart;
  text::Offset m_lEnd;
};

}  // namespace layout

#endif  // EVITA_LAYOUT_LINE_ROOT_INLINE_BOX_H_
