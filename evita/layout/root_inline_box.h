// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_ROOT_INLINE_BOX_H_
#define EVITA_LAYOUT_ROOT_INLINE_BOX_H_

#include <stdint.h>

#include <vector>

#include "evita/ed_defs.h"
#include "evita/gfx/rect_f.h"

namespace views {

class TextView;

namespace rendering {

class InlineBox;
class Formatter;
class TextBlock;
class TextSelection;

class RootInlineBox final {
 public:
  RootInlineBox();
  ~RootInlineBox();

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
  void set_start(text::Posn start) { m_lStart = start; }
  text::Posn text_end() const { return m_lEnd; }
  text::Posn text_start() const { return m_lStart; }

  void AddInlineBox(InlineBox* cell);
  gfx::RectF CalculateSelectionRect(const TextSelection& selection) const;
  RootInlineBox* Copy() const;
  bool Equal(const RootInlineBox*) const;
  void Fix(float ascent, float descent);
  text::Posn GetEnd() const { return m_lEnd; }
  float GetHeight() const { return bounds_.height(); }
  text::Posn GetStart() const { return m_lStart; }
  float GetWidth() const { return bounds_.width(); }
  uint32_t Hash() const;
  gfx::RectF HitTestTextPosition(text::Posn lPosn) const;
  bool IsEndOfDocument() const;
  text::Posn MapXToPosn(float x) const;

 private:
  RootInlineBox(const RootInlineBox& other);

  bool Contains(text::Posn offset) const;

  gfx::RectF bounds_;
  std::vector<InlineBox*> cells_;
  mutable uint32_t m_nHash;
  text::Posn m_lStart;
  text::Posn m_lEnd;
};

}  // namespace rendering
}  // namespace views

#endif  // EVITA_LAYOUT_ROOT_INLINE_BOX_H_
