// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <string>

#include "evita/layout/line/inline_box.h"

#include "base/logging.h"
#include "evita/layout/line/inline_box_visitor.h"
#include "evita/layout/render_font.h"
#include "evita/layout/render_style.h"

namespace layout {

#define V(name) \
  void name::Accept(InlineBoxVisitor* visitor) { visitor->Visit##name(this); }
FOR_EACH_INLINE_BOX(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// InlineBox
//
InlineBox::InlineBox(const RenderStyle& style,
                     float width,
                     float height,
                     float descent)
    : descent_(descent),
      height_(height),
      line_height_(height),
      style_(style),
      width_(width) {
  DCHECK_GE(width, 1.0f);
  DCHECK_GE(height, 1.0f);
}

InlineBox::~InlineBox() {}

float InlineBox::top() const {
  return ::floor(line_height() - line_descent() - height() + descent());
}

void InlineBox::IncrementWidth(float amount) {
  width_ += amount;
}

// InlineBox
text::Offset InlineBox::Fix(float line_height, float line_descent) {
  line_descent_ = line_descent;
  line_height_ = line_height;
  return text::Offset::Invalid();
}

gfx::RectF InlineBox::HitTestTextPosition(text::Offset) const {
  return gfx::RectF();
}

bool InlineBox::Merge(const RenderStyle&, float) {
  return false;
}

//////////////////////////////////////////////////////////////////////
//
// InlineFillerBox
//
InlineFillerBox::InlineFillerBox(const RenderStyle& style,
                                 float width,
                                 float height)
    : InlineBox(style, width, height, 0.0f) {}

InlineFillerBox::~InlineFillerBox() {}

text::Offset InlineFillerBox::MapXToPosn(float x) const {
  return text::Offset::Invalid();
}

//////////////////////////////////////////////////////////////////////
//
// WithFont
//
WithFont::WithFont(const Font& font) : font_(font) {}
WithFont::~WithFont() {}

//////////////////////////////////////////////////////////////////////
//
// InlineMarkerBox
//
InlineMarkerBox::InlineMarkerBox(const RenderStyle& style,
                                 float width,
                                 float height,
                                 text::Offset lPosn,
                                 TextMarker marker_name)
    : InlineBox(style, width, height, style.font().descent()),
      WithFont(style.font()),
      end_(marker_name == TextMarker::LineWrap ? lPosn
                                               : lPosn + text::OffsetDelta(1)),
      marker_name_(marker_name),
      start_(lPosn) {}

InlineMarkerBox::~InlineMarkerBox() {}

// InlineBox
text::Offset InlineMarkerBox::Fix(float line_height, float line_descent) {
  InlineBox::Fix(line_height, line_descent);
  return end_;
}

gfx::RectF InlineMarkerBox::HitTestTextPosition(text::Offset lPosn) const {
  if (lPosn < start_ || lPosn >= end_)
    return gfx::RectF();
  return gfx::RectF(gfx::PointF(0.0f, top()), gfx::SizeF(1.0f, height()));
}

text::Offset InlineMarkerBox::MapXToPosn(float x) const {
  return start_;
}

//////////////////////////////////////////////////////////////////////
//
// InlineTextBoxBase
//
InlineTextBoxBase::InlineTextBoxBase(const RenderStyle& style,
                                     float width,
                                     float height,
                                     text::Offset start,
                                     text::Offset end,
                                     const base::string16& characters)
    : InlineBox(style, width, height, style.font().descent()),
      WithFont(style.font()),
      characters_(characters),
      end_(end),
      start_(start) {}

InlineTextBoxBase::~InlineTextBoxBase() {}

void InlineTextBoxBase::AddChar(base::char16 char_code) {
  characters_.push_back(char_code);
}

void InlineTextBoxBase::ExtendEnd() {
  ++end_;
}

// InlineBox
text::Offset InlineTextBoxBase::Fix(float line_height, float descent) {
  InlineBox::Fix(line_height, descent);
  DCHECK_LT(start_, end_);
  return end_;
}

text::Offset InlineTextBoxBase::MapXToPosn(float x) const {
  if (x >= width())
    return end_;
  for (auto k = 1u; k <= characters_.length(); ++k) {
    auto const cx = style().font().GetTextWidth(characters_.data(), k);
    if (x < cx)
      return start_ + text::OffsetDelta(static_cast<int>(k - 1));
  }
  return end_;
}

//////////////////////////////////////////////////////////////////////
//
// InlineTextBox
//
InlineTextBox::InlineTextBox(const RenderStyle& style,
                             float width,
                             float height,
                             text::Offset start,
                             const base::string16& characters)
    : InlineTextBoxBase(style,
                        width,
                        height,
                        start,
                        start + text::OffsetDelta(characters.size()),
                        characters) {}

InlineTextBox::~InlineTextBox() {}

// Returns bounds rectangle of caret at |offset|. Caret is placed before
// character at |offset|. So, height of caret is height of character before
// |offset|.
gfx::RectF InlineTextBox::HitTestTextPosition(text::Offset offset) const {
  if (offset < start() || offset > end())
    return gfx::RectF();
  auto const length = static_cast<size_t>(offset - start());
  if (length == 0)
    return gfx::RectF(gfx::PointF(0.0f, top()), gfx::SizeF(1.0f, height()));
  auto const width = font().GetTextWidth(characters().data(), length);
  return gfx::RectF(gfx::PointF(::ceil(width), top()),
                    gfx::SizeF(1.0f, height()));
}

bool InlineTextBox::Merge(const RenderStyle& style, float width) {
  if (this->style() != style)
    return false;
  IncrementWidth(width);
  ExtendEnd();
  return true;
}

//////////////////////////////////////////////////////////////////////
//
// InlineUnicodeBox
//
InlineUnicodeBox::InlineUnicodeBox(const RenderStyle& style,
                                   float width,
                                   float height,
                                   text::Offset start,
                                   const base::string16& characters)
    : InlineTextBoxBase(style,
                        width,
                        height,
                        start,
                        start + text::OffsetDelta(1),
                        characters) {}

InlineUnicodeBox::~InlineUnicodeBox() {}

// InlineBox
gfx::RectF InlineUnicodeBox::HitTestTextPosition(text::Offset offset) const {
  if (offset < start() || offset >= end())
    return gfx::RectF();
  return gfx::RectF(gfx::PointF(width(), top()), gfx::SizeF(1.0f, height()));
}

text::Offset InlineUnicodeBox::MapXToPosn(float x) const {
  return start();
}

bool InlineUnicodeBox::Merge(const RenderStyle&, float) {
  return false;
}

}  // namespace layout
