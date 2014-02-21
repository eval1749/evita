// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_render_text_line_h)
#define INCLUDE_evita_views_text_render_text_line_h

#include <vector>

#include "evita/gfx_base.h"
#include "evita/vi_style.h"

namespace views {

class TextRenderer;

namespace rendering {

class Cell;
class DisplayBuffer;
class Formatter;

class TextLine {
  friend class TextRenderer;
  friend class DisplayBuffer;
  friend class rendering::Formatter;

  private: std::vector<Cell*> cells_;
  private: mutable uint m_nHash;
  private: Posn m_lStart;
  private: Posn m_lEnd;
  private: gfx::RectF rect_;

  private: TextLine(const TextLine& other);
  public: TextLine();
  private: ~TextLine();

  public: float bottom() const { return rect_.bottom; }
  public: const std::vector<Cell*>& cells() const { return cells_; }
  public: Cell* last_cell() const { return cells_.back(); }
  public: float left() const { return rect_.left; }
  public: const gfx::RectF& rect() const { return rect_; }
  public: float right() const { return rect_.right; }
  public: float top() const { return rect_.top; }
  public: void set_left_top(const gfx::PointF& left_top);

  public: void AddCell(Cell* cell);
  public: TextLine* Copy() const;
  public: bool Equal(const TextLine*) const;
  public: void Fix(float left, float top, float ascent, float dscent);
  public: Posn GetEnd() const { return m_lEnd; }
  public: float GetHeight() const { return rect_.height(); }
  public: Posn GetStart() const { return m_lStart; }
  public: float GetWidth() const { return rect_.width(); }
  public: uint Hash() const;
  public: Posn MapXToPosn(const gfx::Graphics&, float) const;
  public: void Render(const gfx::Graphics& gfx) const;
  public: void Reset();
};

} // namespace rendering
} // namespace views

#endif //!defined(INCLUDE_evita_views_text_render_text_line_h)
