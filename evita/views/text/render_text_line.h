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
class Formatter;
class TextBlock;
struct TextSelection;

class TextLine final {
  private: gfx::RectF bounds_;
  private: std::vector<Cell*> cells_;
  private: mutable uint m_nHash;
  private: Posn m_lStart;
  private: Posn m_lEnd;

  private: TextLine(const TextLine& other);
  public: TextLine();
  public: ~TextLine();

  public: float bottom() const { return bounds_.bottom; }
  public: const std::vector<Cell*>& cells() const { return cells_; }
  public: Cell* last_cell() const { return cells_.back(); }
  public: float left() const { return bounds_.left; }
  public: const gfx::RectF& bounds() const { return bounds_; }
  public: float right() const { return bounds_.right; }
  public: float top() const { return bounds_.top; }
  public: void set_left_top(const gfx::PointF& left_top);
  public: void set_start(Posn start) { m_lStart = start; }
  public: text::Posn text_end() const { return m_lEnd; }
  public: text::Posn text_start() const { return m_lStart; }

  public: void AddCell(Cell* cell);
  private: bool Contains(text::Posn offset) const;
  public: TextLine* Copy() const;
  public: bool Equal(const TextLine*) const;
  public: void Fix(float left, float top, float ascent, float dscent);
  public: Posn GetEnd() const { return m_lEnd; }
  public: float GetHeight() const { return bounds_.height(); }
  public: Posn GetStart() const { return m_lStart; }
  public: float GetWidth() const { return bounds_.width(); }
  public: uint Hash() const;
  public: gfx::RectF HitTestTextPosition(Posn lPosn) const;
  public: Posn MapXToPosn(const gfx::Canvas&, float) const;
  public: void Render(const gfx::Canvas& gfx) const;
  public: void RenderSelection(gfx::Canvas* canvas,
                               const TextSelection& selection) const;
};

} // namespace rendering
} // namespace views

#endif //!defined(INCLUDE_evita_views_text_render_text_line_h)
