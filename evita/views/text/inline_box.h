// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_TEXT_INLINE_BOX_H_
#define EVITA_VIEWS_TEXT_INLINE_BOX_H_

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

// TODO(eval1749): We should remove |using css::Color|.
// using css::Color;
// TODO(eval1749): We should remove |using css::Style|.
// using css::Style;
// TODO(eval1749): We should remove |using text::TextDecoration|.
// using text::TextDecoration;

class RenderStyle;

//////////////////////////////////////////////////////////////////////
//
// InlineBox
//
class InlineBox : public common::Castable {
  DECLARE_CASTABLE_CLASS(InlineBox, Castable);

 public:
  explicit InlineBox(const InlineBox& other);
  virtual ~InlineBox();

  float descent() const { return descent_; }
  float height() const { return height_; }
  float line_height() const { return line_height_; }
  float width() const { return width_; }

  virtual InlineBox* Copy() const = 0;
  virtual bool Equal(const InlineBox* pInlineBox) const;
  virtual text::Posn Fix(float line_height, float line_descent);
  virtual uint32_t Hash() const;
  virtual gfx::RectF HitTestTextPosition(text::Posn position) const;
  virtual text::Posn MapXToPosn(float x) const;
  virtual bool Merge(const RenderStyle& style, float width);
  virtual void Render(gfx::Canvas* canvas, const gfx::RectF& rect) const;

 protected:
  InlineBox(const RenderStyle& style, float width, float height, float descent);

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
// InlineFillerBox
//
class InlineFillerBox final : public InlineBox {
  DECLARE_CASTABLE_CLASS(InlineFillerBox, InlineBox);

 public:
  InlineFillerBox(const RenderStyle& style, float width, float height);
  InlineFillerBox(const InlineFillerBox& other);
  ~InlineFillerBox() final;

 private:
  // rendering::InlineBox
  InlineBox* Copy() const final;
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
// InlineMarkerBox
//
class InlineMarkerBox final : public InlineBox, private WithFont {
  DECLARE_CASTABLE_CLASS(InlineMarkerBox, InlineBox);

 public:
  InlineMarkerBox(const RenderStyle& style,
                  float width,
                  float height,
                  text::Posn lPosn,
                  TextMarker marker_name);
  InlineMarkerBox(const InlineMarkerBox& other);
  virtual ~InlineMarkerBox();

  TextMarker marker_name() const { return marker_name_; }

 private:
  // rendering::InlineBox
  InlineBox* Copy() const final;
  bool Equal(const InlineBox* pInlineBox) const final;
  text::Posn Fix(float iHeight, float iDescent) final;
  uint32_t Hash() const final;
  gfx::RectF HitTestTextPosition(text::Posn lPosn) const final;
  text::Posn MapXToPosn(float x) const final;
  void Render(gfx::Canvas* canvas, const gfx::RectF& rect) const final;
  text::Posn end_;
  TextMarker marker_name_;
  text::Posn start_;
};

//////////////////////////////////////////////////////////////////////
//
// InlineTextBox
//
class InlineTextBox : public InlineBox, private WithFont {
  DECLARE_CASTABLE_CLASS(InlineTextBox, InlineBox);

 public:
  InlineTextBox(const RenderStyle& style,
                float width,
                float height,
                text::Posn lPosn,
                const base::string16& characters);
  InlineTextBox(const InlineTextBox& other);
  virtual ~InlineTextBox();

  const base::string16 characters() const { return characters_; }
  text::Posn end() const { return end_; }
  text::Posn start() const { return start_; }

  void AddChar(base::char16 char_code);

  bool Equal(const InlineBox* pInlineBox) const override;
  text::Posn Fix(float iHeight, float iDescent) override;
  uint32_t Hash() const override;
  gfx::RectF HitTestTextPosition(text::Posn position) const override;
  text::Posn MapXToPosn(float x) const override;
  bool Merge(const RenderStyle& style, float width) override;
  void Render(gfx::Canvas* canvas, const gfx::RectF& rect) const override;

 private:
  // rendering::InlineBox
  InlineBox* Copy() const override;

  base::string16 characters_;
  text::Posn end_;
  text::Posn start_;
};

//////////////////////////////////////////////////////////////////////
//
// InlineUnicodeBox
//
class InlineUnicodeBox final : public InlineTextBox {
  DECLARE_CASTABLE_CLASS(InlineUnicodeBox, InlineTextBox);

 public:
  InlineUnicodeBox(const RenderStyle& style,
                   float width,
                   float height,
                   text::Posn lPosn,
                   const base::string16& characters);
  InlineUnicodeBox(const InlineUnicodeBox& other);
  ~InlineUnicodeBox() final;

 private:
  // rendering::InlineBox
  InlineBox* Copy() const final;
  gfx::RectF HitTestTextPosition(text::Posn lPosn) const final;
  bool Merge(const RenderStyle& style, float width) final;
  void Render(gfx::Canvas* canvas, const gfx::RectF& rect) const final;
};

}  // namespace rendering
}  // namespace views

#endif  // EVITA_VIEWS_TEXT_INLINE_BOX_H_
