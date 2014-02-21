// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_render_cell_h)
#define INCLUDE_evita_views_text_render_cell_h

#include "common/castable.h"
#include "evita/gfx_base.h"
#include "evita/vi_style.h"

namespace views {
namespace rendering {

//////////////////////////////////////////////////////////////////////
//
// Cell
//
class Cell : public common::Castable {
  DECLARE_CASTABLE_CLASS(Cell, Castable);

  public: float m_cx;
  public: float m_cy;

  protected: Color m_crBackground;

  public: Cell(Color bgcolor, float width, float height);
  public: explicit Cell(const Cell& other);
  public: virtual ~Cell();

  protected: void FillBackground(const gfx::Graphics& gfx,
                                 const gfx::RectF& rect) const;
  public: float GetHeight() const { return m_cy; }
  public: float GetWidth() const { return m_cx; }

  // rendering::Cell
  public: virtual Cell* Copy() const = 0;
  public: virtual bool Equal(const Cell* pCell) const;
  public: virtual Posn Fix(float iHeight, float);
  public: virtual float GetDescent() const;
  public: virtual uint Hash() const;
  public: virtual float MapPosnToX(const gfx::Graphics&, Posn) const;
  public: virtual Posn MapXToPosn(const gfx::Graphics&, float) const;
  public: virtual bool Merge(Font*, Color, Color, TextDecoration, float);
  public: virtual void Render(const gfx::Graphics& gfx,
                               const gfx::RectF& rect) const;
};

//////////////////////////////////////////////////////////////////////
//
// FillerCell
//
class FillerCell final : public Cell {
  DECLARE_CASTABLE_CLASS(FillerCell, Cell);

  public: FillerCell(Color bgcolor, float width, float height);
  public: FillerCell(const FillerCell& other);
  public: virtual ~FillerCell();

  // rendering::Cell
  private: virtual Cell* Copy() const override;
};

}  // namespace rendering
}  // namespace views

#endif //!defined(INCLUDE_evita_views_text_render_cell_h)
