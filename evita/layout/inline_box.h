// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_LAYOUT_INLINE_BOX_H_
#define EVITA_LAYOUT_INLINE_BOX_H_

#include "common/castable.h"
#include "evita/ed_defs.h"
#include "evita/gfx/rect.h"
#include "evita/layout/inline_box_forward.h"
#include "evita/layout/render_style.h"

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
  explicit InlineBox(const InlineBox& other);
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
  virtual text::Posn Fix(float line_height, float line_descent);
  virtual uint32_t Hash() const;
  virtual gfx::RectF HitTestTextPosition(text::Posn position) const;
  virtual text::Posn MapXToPosn(float x) const;
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
};

//////////////////////////////////////////////////////////////////////
//
// InlineFillerBox
//
class InlineFillerBox final : public InlineBox {
  DECLARE_INLINE_BOX_FINAL_CLASS(InlineFillerBox, InlineBox);

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
 public:
  const Font& font() { return *font_; }

 protected:
  explicit WithFont(const Font& font);
  WithFont(const WithFont& other);
  ~WithFont();

 private:
  const Font* font_;
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

  text::Posn end_;
  TextMarker marker_name_;
  text::Posn start_;
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
                    text::Posn lPosn,
                    const base::string16& characters);
  InlineTextBoxBase(const InlineTextBoxBase& other);
  ~InlineTextBoxBase() override;

  text::Posn end() const { return end_; }
  text::Posn start() const { return start_; }

  void ExtendEnd();

 private:
  // InlineBox
  bool Equal(const InlineBox* pInlineBox) const override;
  text::Posn Fix(float iHeight, float iDescent) override;
  uint32_t Hash() const override;
  text::Posn MapXToPosn(float x) const override;

  base::string16 characters_;
  text::Posn end_;
  text::Posn start_;
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
                text::Posn lPosn,
                const base::string16& characters);
  InlineTextBox(const InlineTextBox& other);
  ~InlineTextBox() final;

 private:
  // rendering::InlineBox
  InlineBox* Copy() const final;
  gfx::RectF HitTestTextPosition(text::Posn position) const final;
  bool Merge(const RenderStyle& style, float width) final;
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
                   text::Posn lPosn,
                   const base::string16& characters);
  InlineUnicodeBox(const InlineUnicodeBox& other);
  ~InlineUnicodeBox() final;

 private:
  // rendering::InlineBox
  InlineBox* Copy() const final;
  gfx::RectF HitTestTextPosition(text::Posn lPosn) const final;
  bool Merge(const RenderStyle& style, float width) final;
};

}  // namespace rendering
}  // namespace views

#endif  // EVITA_LAYOUT_INLINE_BOX_H_
