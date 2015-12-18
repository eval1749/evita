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
class InlineBox : public common::Castable {
  DECLARE_INLINE_BOX_ABSTRACT_CLASS(InlineBox, Castable);

 public:
  virtual ~InlineBox();

  float ascent() const { return height_ - descent_; }
  float descent() const { return descent_; }
  float height() const { return height_; }
  float left() const { return left_; }
  const RenderStyle& style() const { return style_; }
  float width() const { return width_; }

  virtual void Accept(InlineBoxVisitor* visitor) = 0;
  virtual gfx::RectF HitTestTextPosition(text::Offset position,
                                         float baseline) const = 0;
  virtual text::Offset MapXToPosn(float x) const = 0;

 protected:
  InlineBox(const RenderStyle& style,
            float left,
            float width,
            float height,
            float descent);

 private:
  const float descent_;
  const float height_;
  const float left_;
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
                  float height);
  ~InlineFillerBox() final;

 private:
  // InlineBox
  gfx::RectF HitTestTextPosition(text::Offset offset,
                                 float baseline) const final;
  text::Offset MapXToPosn(float x) const final;

  DISALLOW_COPY_AND_ASSIGN(InlineFillerBox);
};

//////////////////////////////////////////////////////////////////////
//
// WithFont
//
class WithFont {
 public:
  text::Offset end() const { return end_; }
  const gfx::Font& font() const { return font_; }
  text::Offset start() const { return start_; }

 protected:
  WithFont(const gfx::Font& font, text::Offset start, text::Offset end);
  ~WithFont();

 private:
  const text::Offset end_;
  const gfx::Font& font_;
  const text::Offset start_;

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
                  text::Offset start,
                  TextMarker marker_name);
  virtual ~InlineMarkerBox();

  TextMarker marker_name() const { return marker_name_; }

 private:
  static text::Offset ComputeEndOffset(text::Offset offset,
                                       TextMarker marker_name);

  // InlineBox
  gfx::RectF HitTestTextPosition(text::Offset offset,
                                 float baseline) const final;
  text::Offset MapXToPosn(float x) const final;

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
                    text::Offset start,
                    text::Offset end,
                    const base::string16& characters);
  ~InlineTextBoxBase() override;

 private:
  // InlineBox
  text::Offset MapXToPosn(float x) const override;

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
                text::Offset start,
                const base::string16& characters);
  ~InlineTextBox() final;

 private:
  // InlineBox
  gfx::RectF HitTestTextPosition(text::Offset position,
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
                   text::Offset start,
                   const base::string16& characters);
  ~InlineUnicodeBox() final;

 private:
  // InlineBox
  gfx::RectF HitTestTextPosition(text::Offset offset,
                                 float baseline) const final;
  text::Offset MapXToPosn(float x) const final;

  DISALLOW_COPY_AND_ASSIGN(InlineUnicodeBox);
};

}  // namespace layout

#endif  // EVITA_LAYOUT_LINE_INLINE_BOX_H_
