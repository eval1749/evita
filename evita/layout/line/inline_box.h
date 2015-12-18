// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_LINE_INLINE_BOX_H_
#define EVITA_LAYOUT_LINE_INLINE_BOX_H_

#include "base/macros.h"
#include "common/castable.h"
#include "evita/gfx/rect.h"
#include "evita/layout/line/inline_box_forward.h"
#include "evita/layout/render_style.h"
#include "evita/text/offset.h"

namespace layout {

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

#define DECLARE_INLINE_BOX_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super)

#define DECLARE_INLINE_BOX_ABSTRACT_CLASS(self, super) \
  DECLARE_INLINE_BOX_CLASS(self, super)

#define DECLARE_INLINE_BOX_FINAL_CLASS(self, super) \
  DECLARE_INLINE_BOX_CLASS(self, super)             \
 private:                                           \
  void Accept(InlineBoxVisitor* visitor) final;

//////////////////////////////////////////////////////////////////////
//
// InlineBox
//
class InlineBox : public common::Castable {
  DECLARE_INLINE_BOX_ABSTRACT_CLASS(InlineBox, Castable);

 public:
  virtual ~InlineBox();

  float descent() const { return descent_; }
  float height() const { return height_; }
  float line_height() const { return line_height_; }
  const RenderStyle& style() const { return style_; }
  float top() const;
  float width() const { return width_; }

  virtual void Accept(InlineBoxVisitor* visitor) = 0;
  virtual text::Offset Fix(float line_height, float line_descent);
  virtual gfx::RectF HitTestTextPosition(text::Offset position) const;
  virtual text::Offset MapXToPosn(float x) const = 0;
  virtual bool Merge(const RenderStyle& style, float width);

 protected:
  InlineBox(const RenderStyle& style, float width, float height, float descent);

  float line_descent() const { return line_descent_; }

  void IncrementWidth(float amount);

 private:
  float descent_;
  float height_;
  float line_descent_;
  float line_height_;
  RenderStyle style_;
  float width_;

  DISALLOW_COPY_AND_ASSIGN(InlineBox);
};

//////////////////////////////////////////////////////////////////////
//
// InlineFillerBox
//
// TODO(eval1749): We should use |LayoutBlockFlow|'s padding-left instead of
// |InlineFillerBox|.
class InlineFillerBox final : public InlineBox {
  DECLARE_INLINE_BOX_FINAL_CLASS(InlineFillerBox, InlineBox);

 public:
  InlineFillerBox(const RenderStyle& style, float width, float height);
  ~InlineFillerBox() final;

 private:
  // InlineBox
  text::Offset MapXToPosn(float x) const final;

  DISALLOW_COPY_AND_ASSIGN(InlineFillerBox);
};

//////////////////////////////////////////////////////////////////////
//
// WithFont
//
class WithFont {
 public:
  const Font& font() const { return font_; }

 protected:
  explicit WithFont(const Font& font);
  ~WithFont();

 private:
  const Font& font_;

  DISALLOW_COPY_AND_ASSIGN(WithFont);
};

//////////////////////////////////////////////////////////////////////
//
// InlineMarkerBox
//
class InlineMarkerBox final : public InlineBox, public WithFont {
  DECLARE_INLINE_BOX_FINAL_CLASS(InlineMarkerBox, InlineBox);

 public:
  InlineMarkerBox(const RenderStyle& style,
                  float width,
                  float height,
                  text::Offset lPosn,
                  TextMarker marker_name);
  virtual ~InlineMarkerBox();

  TextMarker marker_name() const { return marker_name_; }

 private:
  // InlineBox
  text::Offset Fix(float iHeight, float iDescent) final;
  gfx::RectF HitTestTextPosition(text::Offset lPosn) const final;
  text::Offset MapXToPosn(float x) const final;

  text::Offset end_;
  TextMarker marker_name_;
  text::Offset start_;

  DISALLOW_COPY_AND_ASSIGN(InlineMarkerBox);
};

//////////////////////////////////////////////////////////////////////
//
// InlineTextBoxBase
//
class InlineTextBoxBase : public InlineBox, public WithFont {
  DECLARE_INLINE_BOX_ABSTRACT_CLASS(InlineTextBoxBase, InlineBox);

 public:
  const base::string16 characters() const { return characters_; }

  void AddChar(base::char16 char_code);

 protected:
  InlineTextBoxBase(const RenderStyle& style,
                    float width,
                    float height,
                    text::Offset lPosn,
                    const base::string16& characters);
  ~InlineTextBoxBase() override;

  text::Offset end() const { return end_; }
  text::Offset start() const { return start_; }

  void ExtendEnd();

 private:
  // InlineBox
  text::Offset Fix(float iHeight, float iDescent) override;
  text::Offset MapXToPosn(float x) const override;

  base::string16 characters_;
  text::Offset end_;
  text::Offset start_;

  DISALLOW_COPY_AND_ASSIGN(InlineTextBoxBase);
};

//////////////////////////////////////////////////////////////////////
//
// InlineTextBox
//
class InlineTextBox final : public InlineTextBoxBase {
  DECLARE_INLINE_BOX_FINAL_CLASS(InlineTextBox, InlineTextBoxBase);

 public:
  InlineTextBox(const RenderStyle& style,
                float width,
                float height,
                text::Offset lPosn,
                const base::string16& characters);
  ~InlineTextBox() final;

 private:
  // InlineBox
  gfx::RectF HitTestTextPosition(text::Offset position) const final;
  bool Merge(const RenderStyle& style, float width) final;

  DISALLOW_COPY_AND_ASSIGN(InlineTextBox);
};

//////////////////////////////////////////////////////////////////////
//
// InlineUnicodeBox
//
class InlineUnicodeBox final : public InlineTextBoxBase {
  DECLARE_INLINE_BOX_FINAL_CLASS(InlineUnicodeBox, InlineTextBoxBase);

 public:
  InlineUnicodeBox(const RenderStyle& style,
                   float width,
                   float height,
                   text::Offset lPosn,
                   const base::string16& characters);
  ~InlineUnicodeBox() final;

 private:
  // InlineBox
  gfx::RectF HitTestTextPosition(text::Offset lPosn) const final;
  bool Merge(const RenderStyle& style, float width) final;

  DISALLOW_COPY_AND_ASSIGN(InlineUnicodeBox);
};

}  // namespace layout

#endif  // EVITA_LAYOUT_LINE_INLINE_BOX_H_
