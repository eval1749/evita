// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_text_render_cell_h)
#define INCLUDE_evita_views_text_render_cell_h

#include "common/castable.h"
#include "evita/gfx_base.h"
#include "evita/views/text/render_style.h"

namespace views {
namespace rendering {

enum class TextMarker{
  EndOfDocument,
  EndOfLine,
  LineWrap,
  Tab,
};

// TODO(yosi) We should remove |using css::Color|.
//using css::Color;
// TODO(yosi) We should remove |using css::Style|.
//using css::Style;
// TODO(yosi) We should remove |using text::TextDecoration|.
//using text::TextDecoration;

class RenderStyle;

//////////////////////////////////////////////////////////////////////
//
// Cell
//
class Cell : public common::Castable {
  DECLARE_CASTABLE_CLASS(Cell, Castable);

  public: float m_cx;
  public: float m_cy;
  private: RenderStyle style_;

  public: Cell(const RenderStyle& style, float width, float height);
  public: explicit Cell(const Cell& other);
  public: virtual ~Cell();

  protected: const RenderStyle& style() const { return style_; }

  protected: void FillOverlay(const gfx::Graphics& gfx,
                              const gfx::RectF& rect) const;
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
  public: virtual bool Merge(const RenderStyle& style, float width);
  public: virtual void Render(const gfx::Graphics& gfx,
                               const gfx::RectF& rect) const;
};

//////////////////////////////////////////////////////////////////////
//
// FillerCell
//
class FillerCell final : public Cell {
  DECLARE_CASTABLE_CLASS(FillerCell, Cell);

  public: FillerCell(const RenderStyle& style, float width, float height);
  public: FillerCell(const FillerCell& other);
  public: virtual ~FillerCell();

  // rendering::Cell
  private: virtual Cell* Copy() const override;
};

//////////////////////////////////////////////////////////////////////
//
// MarkerCell
//
class MarkerCell final : public Cell {
  DECLARE_CASTABLE_CLASS(MarkerCell, Cell);

  private: Posn m_lStart;
  private: Posn m_lEnd;
  private: float m_iAscent;
  private: float m_iDescent;
  private: TextMarker marker_name_;

  public: MarkerCell(const RenderStyle& style, float width, float height,
                     Posn lPosn, TextMarker marker_name);
  public: MarkerCell(const MarkerCell& other);
  public: ~MarkerCell();

  public: TextMarker marker_name() const { return marker_name_; }

  // rendering::Cell
  private: virtual Cell* Copy() const override;
  private: virtual bool Equal(const Cell* pCell) const override;
  private: virtual Posn Fix(float iHeight, float iDescent) override;
  private: virtual float GetDescent() const override;
  private: virtual uint Hash() const override;
  private: virtual float MapPosnToX(const gfx::Graphics&,
                                    Posn lPosn) const override;
  private: virtual Posn MapXToPosn(const gfx::Graphics& gfx,
                                   float x) const override;
  private: virtual void Render(const gfx::Graphics& gfx,
                               const gfx::RectF& rect) const override;
};

//////////////////////////////////////////////////////////////////////
//
// TextCell
//
class TextCell : public Cell {
  DECLARE_CASTABLE_CLASS(TextCell, Cell);

  private: float m_iDescent;

  private: Posn m_lStart;
  private: Posn m_lEnd;

  private: base::string16 characters_;

  public: TextCell(const RenderStyle& style, float width, float height,
                   Posn lPosn, const base::string16& characters);
  public: TextCell(const TextCell& other);
  public: virtual ~TextCell();

  public: const base::string16 characters() const { return characters_; }

  public: void AddChar(base::char16 char_code);

  // rendering::Cell
  private: virtual Cell* Copy() const override;
  public: virtual bool Equal(const Cell* pCell) const override;
  public: virtual Posn Fix(float iHeight, float iDescent) override;
  public: virtual float GetDescent() const override;
  public: virtual uint Hash() const override final;
  public: virtual float MapPosnToX(const gfx::Graphics& gfx,
                                   Posn lPosn) const override final;
  public: virtual Posn MapXToPosn(const gfx::Graphics& gfx,
                                  float x) const override final;
  public: virtual bool Merge(const RenderStyle& style, float width) override;
  public: virtual void Render(const gfx::Graphics& gfx,
                              const gfx::RectF& rect) const override;
};

//////////////////////////////////////////////////////////////////////
//
// UnicodeCell
//
class UnicodeCell final : public TextCell {
  DECLARE_CASTABLE_CLASS(UnicodeCell, TextCell);

  public: UnicodeCell(const RenderStyle& style, float width, float height,
                      Posn lPosn, const base::string16& characters);
  public: UnicodeCell(const UnicodeCell& other);
  public: virtual ~UnicodeCell();

  // rendering::Cell
  private: virtual Cell* Copy() const override;
  private: virtual void Render(const gfx::Graphics& gfx,
                               const gfx::RectF& rect) const override;
};

} // namespace rendering
} // namespace views

#endif //!defined(INCLUDE_evita_views_text_render_cell_h)
