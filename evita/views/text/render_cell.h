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

//////////////////////////////////////////////////////////////////////
//
// MarkerCell
//
class MarkerCell final : public Cell {
  DECLARE_CASTABLE_CLASS(MarkerCell, Cell);

  public: enum Kind {
    Kind_Eob,
    Kind_Eol,
    Kind_Tab,
    Kind_Wrap,
  };

  private: Posn m_lStart;
  private: Posn m_lEnd;
  private: Color m_crColor;
  private: float m_iAscent;
  private: float m_iDescent;
  private: Kind m_eKind;

  public: MarkerCell(Color crColor,
                     Color crBackground,
                     float cx,
                     float iHeight,
                     float iDescent,
                     Posn lPosn,
                     Kind eKind);
  public: MarkerCell(const MarkerCell& other);
  public: ~MarkerCell();

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

  private: Color m_crColor;
  private: TextDecoration m_eDecoration;
  private: float m_iDescent;

  private: Posn m_lStart;
  private: Posn m_lEnd;

  private: Font* m_pFont;
  private: base::string16 characters_;

  public: TextCell(const gfx::Graphics& gfx, const StyleValues* pStyle,
                   Color crColor, Color crBackground, Font* pFont, float cx,
                   Posn lPosn, const base::string16& characters);
  public: TextCell(const TextCell& other);
  public: virtual ~TextCell();

  public: const base::string16 characters() const { return characters_; }
  public: Color color() const { return m_crColor; }
  public: Font* font() const { return m_pFont; }

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
  public: virtual bool Merge(Font* pFont, Color crColor, Color crBackground,
                             TextDecoration eDecoration, float cx) override;
  public: virtual void Render(const gfx::Graphics& gfx,
                              const gfx::RectF& rect) const override;
};

//////////////////////////////////////////////////////////////////////
//
// UnicodeCell
//
class UnicodeCell final : public TextCell {
  DECLARE_CASTABLE_CLASS(UnicodeCell, TextCell);

  public: UnicodeCell(const gfx::Graphics& gfx, const StyleValues* pStyle,
                      Color crColor, Color crBackground, Font* pFont,
                      float cx, Posn lPosn,
                      const base::string16& characters);
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
