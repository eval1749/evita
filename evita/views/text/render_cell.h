// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TEXT_RENDER_CELL_H_
#define EVITA_VIEWS_TEXT_RENDER_CELL_H_

#include "common/castable.h"
#include "evita/gfx_base.h"
#include "evita/precomp.h"
#include "evita/views/text/render_style.h"

namespace views {
namespace rendering {

enum class TextMarker {
  EndOfDocument,
  EndOfLine,
  LineWrap,
  Tab,
};

// TODO(yosi) We should remove |using css::Color|.
// using css::Color;
// TODO(yosi) We should remove |using css::Style|.
// using css::Style;
// TODO(yosi) We should remove |using text::TextDecoration|.
// using text::TextDecoration;

class RenderStyle;

//////////////////////////////////////////////////////////////////////
//
// Cell
//
class Cell : public common::Castable {
  DECLARE_CASTABLE_CLASS(Cell, Castable);

 public:
  explicit Cell(const Cell& other);
  virtual ~Cell();

  float descent() const { return descent_; }
  float height() const { return height_; }
  float line_height() const { return line_height_; }
  float width() const { return width_; }

  virtual Cell* Copy() const = 0;
  virtual bool Equal(const Cell* pCell) const;
  virtual Posn Fix(float line_height, float line_descent);
  virtual uint32_t Hash() const;
  virtual gfx::RectF HitTestTextPosition(Posn position) const;
  virtual Posn MapXToPosn(float x) const;
  virtual bool Merge(const RenderStyle& style, float width);
  virtual void Render(gfx::Canvas* canvas, const gfx::RectF& rect) const;

 protected:
  Cell(const RenderStyle& style, float width, float height, float descent);

  float line_descent() const { return line_descent_; }
  float top() const;
  const RenderStyle& style() const { return style_; }

  void FillOverlay(gfx::Canvas* canvas, const gfx::RectF& rect) const;
  void FillBackground(gfx::Canvas* canvas, const gfx::RectF& rect) const;
  void IncrementWidth(float amount);

 private:
  float descent_;
  float height_;
  float line_descent_;
  float line_height_;
  RenderStyle style_;
  float width_;
};

//////////////////////////////////////////////////////////////////////
//
// FillerCell
//
class FillerCell final : public Cell {
  DECLARE_CASTABLE_CLASS(FillerCell, Cell);

 public:
  FillerCell(const RenderStyle& style, float width, float height);
  FillerCell(const FillerCell& other);
  ~FillerCell() final;

 private:
  // rendering::Cell
  Cell* Copy() const final;
};

//////////////////////////////////////////////////////////////////////
//
// WithFont
//
class WithFont {
 protected:
  explicit WithFont(const Font& font);
  WithFont(const WithFont& other);
  ~WithFont();

  float underline() const;
  float underline_thickness() const;

  void DrawHLine(gfx::Canvas* canvas,
                 const gfx::Brush& brush,
                 float sx,
                 float sy,
                 float y) const;
  void DrawLine(gfx::Canvas* canvas,
                const gfx::Brush& brush,
                float x1,
                float y1,
                float x2,
                float y2,
                float width) const;
  void DrawVLine(gfx::Canvas* canvas,
                 const gfx::Brush& brush,
                 float x,
                 float sy,
                 float ey) const;
  void DrawWave(gfx::Canvas* canvas,
                const gfx::Brush& brush,
                const gfx::RectF& bounds,
                float baseline) const;

 private:
  const Font* font_;
};

//////////////////////////////////////////////////////////////////////
//
// MarkerCell
//
class MarkerCell final : public Cell, private WithFont {
  DECLARE_CASTABLE_CLASS(MarkerCell, Cell);

 public:
  MarkerCell(const RenderStyle& style,
             float width,
             float height,
             Posn lPosn,
             TextMarker marker_name);
  MarkerCell(const MarkerCell& other);
  virtual ~MarkerCell();

  TextMarker marker_name() const { return marker_name_; }

 private:
  // rendering::Cell
  Cell* Copy() const final;
  bool Equal(const Cell* pCell) const final;
  Posn Fix(float iHeight, float iDescent) final;
  uint32_t Hash() const final;
  gfx::RectF HitTestTextPosition(Posn lPosn) const final;
  Posn MapXToPosn(float x) const final;
  void Render(gfx::Canvas* canvas, const gfx::RectF& rect) const final;
  Posn end_;
  TextMarker marker_name_;
  Posn start_;
};

//////////////////////////////////////////////////////////////////////
//
// TextCell
//
class TextCell : public Cell, private WithFont {
  DECLARE_CASTABLE_CLASS(TextCell, Cell);

 public:
  TextCell(const RenderStyle& style,
           float width,
           float height,
           Posn lPosn,
           const base::string16& characters);
  TextCell(const TextCell& other);
  virtual ~TextCell();

  const base::string16 characters() const { return characters_; }
  Posn end() const { return end_; }
  Posn start() const { return start_; }

  void AddChar(base::char16 char_code);

  bool Equal(const Cell* pCell) const override;
  Posn Fix(float iHeight, float iDescent) override;
  uint32_t Hash() const override;
  gfx::RectF HitTestTextPosition(Posn position) const override;
  Posn MapXToPosn(float x) const override;
  bool Merge(const RenderStyle& style, float width) override;
  void Render(gfx::Canvas* canvas, const gfx::RectF& rect) const override;

 private:
  // rendering::Cell
  Cell* Copy() const override;

  base::string16 characters_;
  Posn end_;
  Posn start_;
};

//////////////////////////////////////////////////////////////////////
//
// UnicodeCell
//
class UnicodeCell final : public TextCell {
  DECLARE_CASTABLE_CLASS(UnicodeCell, TextCell);

 public:
  UnicodeCell(const RenderStyle& style,
              float width,
              float height,
              Posn lPosn,
              const base::string16& characters);
  UnicodeCell(const UnicodeCell& other);
  ~UnicodeCell() final;

 private:
  // rendering::Cell
  Cell* Copy() const final;
  gfx::RectF HitTestTextPosition(Posn lPosn) const final;
  bool Merge(const RenderStyle& style, float width) final;
  void Render(gfx::Canvas* canvas, const gfx::RectF& rect) const final;
};

}  // namespace rendering
}  // namespace views

#endif  // EVITA_VIEWS_TEXT_RENDER_CELL_H_
