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

  private: float descent_;
  private: float height_;
  private: float line_descent_;
  private: float line_height_;
  private: RenderStyle style_;
  private: float width_;

  protected: Cell(const RenderStyle& style, float width, float height,
                  float descent);
  public: explicit Cell(const Cell& other);
  public: virtual ~Cell();

  public: float descent() const { return descent_; }
  public: float height() const { return height_; }
  protected: float line_descent() const { return line_descent_; }
  public: float line_height() const { return line_height_; }
  protected: float top() const;
  public: float width() const { return width_; }

  protected: const RenderStyle& style() const { return style_; }

  protected: void FillOverlay(const gfx::Graphics& gfx,
                              const gfx::RectF& rect) const;
  protected: void FillBackground(const gfx::Graphics& gfx,
                                 const gfx::RectF& rect) const;
  protected: void IncrementWidth(float amount);

  // rendering::Cell
  public: virtual Cell* Copy() const = 0;
  public: virtual bool Equal(const Cell* pCell) const;
  public: virtual Posn Fix(float line_height, float line_descent);
  public: virtual uint32_t Hash() const;
  public: virtual gfx::RectF HitTestTextPosition(Posn position) const;
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

  private: Posn start_;
  private: Posn end_;
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
  private: virtual uint32_t Hash() const override;
  private: virtual gfx::RectF HitTestTextPosition(Posn lPosn) const override;
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

  private: Posn start_;
  private: Posn end_;
  private: base::string16 characters_;

  public: TextCell(const RenderStyle& style, float width, float height,
                   Posn lPosn, const base::string16& characters);
  public: TextCell(const TextCell& other);
  public: virtual ~TextCell();

  public: const base::string16 characters() const { return characters_; }
  public: Posn end() const { return end_; }
  public: Posn start() const { return start_; }

  public: void AddChar(base::char16 char_code);

  // rendering::Cell
  private: virtual Cell* Copy() const override;
  public: virtual bool Equal(const Cell* pCell) const override;
  public: virtual Posn Fix(float iHeight, float iDescent) override;
  public: virtual uint32_t Hash() const override final;
  public: virtual gfx::RectF HitTestTextPosition(
      Posn position) const override;
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
  private: virtual gfx::RectF HitTestTextPosition(Posn lPosn) const override;
  private: virtual bool Merge(const RenderStyle& style, float width) override;
  private: virtual void Render(const gfx::Graphics& gfx,
                               const gfx::RectF& rect) const override;
};

} // namespace rendering
} // namespace views

#endif //!defined(INCLUDE_evita_views_text_render_cell_h)
