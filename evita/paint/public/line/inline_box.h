// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_PAINT_PUBLIC_LINE_INLINE_BOX_H_
#define EVITA_PAINT_PUBLIC_LINE_INLINE_BOX_H_

#include "base/macros.h"
#include "common/castable.h"
#include "evita/gfx/rect.h"
#include "evita/layout/render_style.h"
#include "evita/paint/public/line/inline_box_forward.h"

namespace layout {
class Font;
class RenderStyle;
}

namespace paint {

enum class TextMarker {
  EndOfDocument,
  EndOfLine,
  LineWrap,
  Tab,
};

#define DECLARE_PAINT_INLINE_BOX_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super)

#define DECLARE_PAINT_INLINE_BOX_ABSTRACT_CLASS(self, super) \
  DECLARE_PAINT_INLINE_BOX_CLASS(self, super)

#define DECLARE_PAINT_INLINE_BOX_FINAL_CLASS(self, super) \
  DECLARE_PAINT_INLINE_BOX_CLASS(self, super)             \
 private:                                                 \
  void Accept(InlineBoxVisitor* visitor) final;

//////////////////////////////////////////////////////////////////////
//
// InlineBox
//
class InlineBox : public common::Castable {
  DECLARE_PAINT_INLINE_BOX_ABSTRACT_CLASS(InlineBox, Castable);

 public:
  using RenderStyle = layout::RenderStyle;

  virtual ~InlineBox();

  float descent() const { return descent_; }
  float height() const { return height_; }
  float line_height() const { return line_height_; }
  const RenderStyle& style() const { return style_; }
  float top() const;
  float width() const { return width_; }

  virtual void Accept(InlineBoxVisitor* visitor) = 0;
  virtual InlineBox* Copy() const = 0;
  virtual bool Equal(const InlineBox* pInlineBox) const;
  virtual size_t Hash() const;

 protected:
  InlineBox(const RenderStyle& style,
            float width,
            float height,
            float descent,
            float line_height,
            float line_descent);

  float line_descent() const { return line_descent_; }

 private:
  const float descent_;
  const float height_;
  const float line_descent_;
  const float line_height_;
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
  DECLARE_PAINT_INLINE_BOX_FINAL_CLASS(InlineFillerBox, InlineBox);

 public:
  InlineFillerBox(const RenderStyle& style,
                  float width,
                  float height,
                  float line_height,
                  float line_descent);
  ~InlineFillerBox() final;

 private:
  // InlineBox
  InlineBox* Copy() const final;

  DISALLOW_COPY_AND_ASSIGN(InlineFillerBox);
};

//////////////////////////////////////////////////////////////////////
//
// WithFont
//
class WithFont {
 public:
  // We should use |gfx::Font| instead of |layout::Font|.
  using Font = layout::Font;

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
  DECLARE_PAINT_INLINE_BOX_FINAL_CLASS(InlineMarkerBox, InlineBox);

 public:
  InlineMarkerBox(const RenderStyle& style,
                  float width,
                  float height,
                  TextMarker marker_name,
                  float line_height,
                  float line_descent);
  ~InlineMarkerBox() final;

  TextMarker marker_name() const { return marker_name_; }

 private:
  // InlineBox
  InlineBox* Copy() const final;
  bool Equal(const InlineBox* pInlineBox) const final;
  size_t Hash() const final;

  const TextMarker marker_name_;

  DISALLOW_COPY_AND_ASSIGN(InlineMarkerBox);
};

//////////////////////////////////////////////////////////////////////
//
// InlineTextBoxBase
//
class InlineTextBoxBase : public InlineBox, public WithFont {
  DECLARE_PAINT_INLINE_BOX_ABSTRACT_CLASS(InlineTextBoxBase, InlineBox);

 public:
  const base::string16 characters() const { return characters_; }

 protected:
  InlineTextBoxBase(const RenderStyle& style,
                    float width,
                    float height,
                    const base::string16& characters,
                    float line_height,
                    float line_descent);
  ~InlineTextBoxBase() override;

 private:
  // InlineBox
  bool Equal(const InlineBox* pInlineBox) const override;
  size_t Hash() const override;

  const base::string16 characters_;

  DISALLOW_COPY_AND_ASSIGN(InlineTextBoxBase);
};

//////////////////////////////////////////////////////////////////////
//
// InlineTextBox
//
class InlineTextBox final : public InlineTextBoxBase {
  DECLARE_PAINT_INLINE_BOX_FINAL_CLASS(InlineTextBox, InlineTextBoxBase);

 public:
  InlineTextBox(const RenderStyle& style,
                float width,
                float height,
                const base::string16& characters,
                float line_height,
                float line_descent);
  ~InlineTextBox() final;

 private:
  // InlineBox
  InlineBox* Copy() const final;

  DISALLOW_COPY_AND_ASSIGN(InlineTextBox);
};

//////////////////////////////////////////////////////////////////////
//
// InlineUnicodeBox
//
class InlineUnicodeBox final : public InlineTextBoxBase {
  DECLARE_PAINT_INLINE_BOX_FINAL_CLASS(InlineUnicodeBox, InlineTextBoxBase);

 public:
  InlineUnicodeBox(const RenderStyle& style,
                   float width,
                   float height,
                   const base::string16& characters,
                   float line_height,
                   float line_descent);
  ~InlineUnicodeBox() final;

 private:
  // InlineBox
  InlineBox* Copy() const final;

  DISALLOW_COPY_AND_ASSIGN(InlineUnicodeBox);
};

}  // namespace paint

#endif  // EVITA_PAINT_PUBLIC_LINE_INLINE_BOX_H_
