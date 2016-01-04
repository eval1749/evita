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

namespace gfx {
class Font;
}

namespace layout {

enum class TextMarker {
  EndOfDocument,
  EndOfLine,
  LineWrap,
  Tab,
};

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
class InlineBox : public common::Castable<InlineBox> {
  DECLARE_INLINE_BOX_ABSTRACT_CLASS(InlineBox, Castable);

 public:
  virtual ~InlineBox();

  float ascent() const { return height_ - descent_; }
  float descent() const { return descent_; }
  text::OffsetDelta end() const { return end_; }
  float height() const { return height_; }
  float left() const { return left_; }
  text::OffsetDelta start() const { return start_; }
  const RenderStyle& style() const { return style_; }
  float width() const { return width_; }

  virtual void Accept(InlineBoxVisitor* visitor) = 0;

  // Returns text offset at |point_x| in box local coordinate, or |start()|
  // for negative |point_x|, or |end()| for |point_x| grater than or equal to
  // |width()|. This function never return |text::OffsetDelta::Invalid()|.
  virtual text::OffsetDelta HitTestPoint(float point_x) const;
  virtual gfx::RectF HitTestTextPosition(text::OffsetDelta position,
                                         float baseline) const = 0;

 protected:
  InlineBox(const RenderStyle& style,
            float left,
            float width,
            float height,
            text::OffsetDelta start,
            text::OffsetDelta end,
            float descent);

 private:
  const float descent_;
  const text::OffsetDelta end_;
  const float height_;
  const float left_;
  const text::OffsetDelta start_;
  const RenderStyle style_;
  const float width_;

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
  InlineFillerBox(const RenderStyle& style,
                  float left,
                  float width,
                  float height,
                  text::OffsetDelta offset);
  ~InlineFillerBox() final;

 private:
  // InlineBox
  gfx::RectF HitTestTextPosition(text::OffsetDelta offset,
                                 float baseline) const final;
  DISALLOW_COPY_AND_ASSIGN(InlineFillerBox);
};

//////////////////////////////////////////////////////////////////////
//
// WithFont
//
class WithFont {
 public:
  const gfx::Font& font() const { return font_; }

 protected:
  explicit WithFont(const gfx::Font& font);
  ~WithFont();

 private:
  const gfx::Font& font_;

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
                  float left,
                  float width,
                  float height,
                  text::OffsetDelta start,
                  text::OffsetDelta end,
                  TextMarker marker_name);
  virtual ~InlineMarkerBox();

  TextMarker marker_name() const { return marker_name_; }

 private:
  static text::OffsetDelta ComputeEndOffset(text::OffsetDelta offset,
                                            TextMarker marker_name);

  // InlineBox
  gfx::RectF HitTestTextPosition(text::OffsetDelta offset,
                                 float baseline) const final;

  const TextMarker marker_name_;

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

 protected:
  InlineTextBoxBase(const RenderStyle& style,
                    float left,
                    float width,
                    float height,
                    text::OffsetDelta start,
                    text::OffsetDelta end,
                    const base::string16& characters);
  ~InlineTextBoxBase() override;

 private:
  // InlineBox
  text::OffsetDelta HitTestPoint(float x) const override;

  const base::string16 characters_;

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
                float left,
                float width,
                float height,
                text::OffsetDelta start,
                const base::string16& characters);
  ~InlineTextBox() final;

 private:
  // InlineBox
  gfx::RectF HitTestTextPosition(text::OffsetDelta position,
                                 float baseline) const final;

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
                   float left,
                   float width,
                   float height,
                   text::OffsetDelta start,
                   const base::string16& characters);
  ~InlineUnicodeBox() final;

 private:
  // InlineBox
  text::OffsetDelta HitTestPoint(float x) const override;
  gfx::RectF HitTestTextPosition(text::OffsetDelta offset,
                                 float baseline) const final;

  DISALLOW_COPY_AND_ASSIGN(InlineUnicodeBox);
};

}  // namespace layout

#endif  // EVITA_LAYOUT_LINE_INLINE_BOX_H_
