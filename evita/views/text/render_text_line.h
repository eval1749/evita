// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TEXT_RENDER_TEXT_LINE_H_
#define EVITA_VIEWS_TEXT_RENDER_TEXT_LINE_H_

#include <stdint.h>

#include <vector>

#include "evita/gfx_base.h"
#include "evita/precomp.h"

namespace views {

class TextRenderer;

namespace rendering {

class Cell;
class Formatter;
class TextBlock;
class TextSelection;

class TextLine final {
 public:
  TextLine();
  ~TextLine();

  float bottom() const { return bounds_.bottom; }
  const gfx::PointF& bottom_right() const { return bounds_.bottom_right(); }
  const gfx::RectF& bounds() const { return bounds_; }
  const std::vector<Cell*>& cells() const { return cells_; }
  float height() const { return bounds_.height(); }
  Cell* last_cell() const { return cells_.back(); }
  float left() const { return bounds_.left; }
  float right() const { return bounds_.right; }
  float top() const { return bounds_.top; }
  const gfx::PointF origin() const { return bounds_.origin(); }
  void set_origin(const gfx::PointF& origin);
  void set_start(Posn start) { m_lStart = start; }
  text::Posn text_end() const { return m_lEnd; }
  text::Posn text_start() const { return m_lStart; }

  void AddCell(Cell* cell);
  gfx::RectF CalculateSelectionRect(const TextSelection& selection) const;
  TextLine* Copy() const;
  bool Equal(const TextLine*) const;
  void Fix(float ascent, float descent);
  Posn GetEnd() const { return m_lEnd; }
  float GetHeight() const { return bounds_.height(); }
  Posn GetStart() const { return m_lStart; }
  float GetWidth() const { return bounds_.width(); }
  uint Hash() const;
  gfx::RectF HitTestTextPosition(Posn lPosn) const;
  Posn MapXToPosn(float x) const;
  void Render(gfx::Canvas* canvas) const;

 private:
  TextLine(const TextLine& other);

  bool Contains(text::Posn offset) const;

  gfx::RectF bounds_;
  std::vector<Cell*> cells_;
  mutable uint32_t m_nHash;
  Posn m_lStart;
  Posn m_lEnd;
};

}  // namespace rendering
}  // namespace views

#endif  // EVITA_VIEWS_TEXT_RENDER_TEXT_LINE_H_
