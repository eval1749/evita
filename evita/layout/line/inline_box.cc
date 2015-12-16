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

InlineBox::InlineBox(const InlineBox& other)
    : InlineBox(other.style_, other.width_, other.height_, other.descent_) {
  line_descent_ = other.line_descent_;
  line_height_ = other.line_height_;
}

InlineBox::~InlineBox() {}

float InlineBox::top() const {
  return line_height() - line_descent() - height() + descent();
}

void InlineBox::IncrementWidth(float amount) {
  width_ += amount;
}

// InlineBox
bool InlineBox::Equal(const InlineBox* other) const {
  return other->class_name() == class_name() && other->width_ == width_ &&
         other->line_height_ == line_height_ && other->style_ == style_;
}

text::Offset InlineBox::Fix(float line_height, float line_descent) {
  line_descent_ = line_descent;
  line_height_ = line_height;
  return text::Offset::Invalid();
}

uint32_t InlineBox::Hash() const {
  auto nHash = static_cast<uint32_t>(width_);
  nHash ^= static_cast<uint32_t>(line_height_);
  nHash ^= std::hash<RenderStyle>()(style_);
  return nHash;
}

gfx::RectF InlineBox::HitTestTextPosition(text::Offset) const {
  return gfx::RectF();
}

text::Offset InlineBox::MapXToPosn(float x) const {
  return text::Offset::Invalid();
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

InlineFillerBox::InlineFillerBox(const InlineFillerBox& other)
    : InlineBox(other) {}

InlineFillerBox::~InlineFillerBox() {}

InlineBox* InlineFillerBox::Copy() const {
  return new InlineFillerBox(*this);
}

//////////////////////////////////////////////////////////////////////
//
// WithFont
//
WithFont::WithFont(const Font& font) : font_(&font) {}

WithFont::WithFont(const WithFont& other) : font_(other.font_) {}

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

InlineMarkerBox::InlineMarkerBox(const InlineMarkerBox& other)
    : InlineBox(other),
      WithFont(other),
      end_(other.end_),
      marker_name_(other.marker_name_),
      start_(other.start_) {}

InlineMarkerBox::~InlineMarkerBox() {}

// InlineBox
InlineBox* InlineMarkerBox::Copy() const {
  return new InlineMarkerBox(*this);
}

bool InlineMarkerBox::Equal(const InlineBox* other) const {
  if (!InlineBox::Equal(other))
    return false;
  auto const marker_cell = other->as<InlineMarkerBox>();
  return marker_name_ == marker_cell->marker_name_;
}

text::Offset InlineMarkerBox::Fix(float line_height, float line_descent) {
  InlineBox::Fix(line_height, line_descent);
  return end_;
}

uint32_t InlineMarkerBox::Hash() const {
  auto nHash = InlineBox::Hash();
  nHash <<= 8;
  nHash ^= static_cast<int>(marker_name_);
  return nHash;
}

gfx::RectF InlineMarkerBox::HitTestTextPosition(text::Offset lPosn) const {
  if (lPosn < start_ || lPosn >= end_)
    return gfx::RectF();
  return gfx::RectF(gfx::PointF(0.0f, top()), gfx::SizeF(width(), height()));
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
                                     text::Offset lPosn,
                                     const base::string16& characters)
    : InlineBox(style, width, height, style.font().descent()),
      WithFont(style.font()),
      characters_(characters),
      end_(lPosn + text::OffsetDelta(1)),
      start_(lPosn) {}

InlineTextBoxBase::InlineTextBoxBase(const InlineTextBoxBase& other)
    : InlineBox(other),
      WithFont(other),
      characters_(other.characters_),
      end_(other.end_),
      start_(other.start_) {}

InlineTextBoxBase::~InlineTextBoxBase() {}

void InlineTextBoxBase::AddChar(base::char16 char_code) {
  characters_.push_back(char_code);
}

void InlineTextBoxBase::ExtendEnd() {
  ++end_;
}

// InlineBox
bool InlineTextBoxBase::Equal(const InlineBox* other) const {
  if (!InlineBox::Equal(other))
    return false;
  return characters_ == other->as<InlineTextBox>()->characters_;
}

text::Offset InlineTextBoxBase::Fix(float line_height, float descent) {
  InlineBox::Fix(line_height, descent);
  DCHECK_LT(start_, end_);
  return end_;
}

uint32_t InlineTextBoxBase::Hash() const {
  return static_cast<uint32_t>((InlineBox::Hash() << 3) ^
                               std::hash<base::string16>()(characters_));
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
                             text::Offset offset,
                             const base::string16& characters)
    : InlineTextBoxBase(style, width, height, offset, characters) {}

InlineTextBox::InlineTextBox(const InlineTextBox& other)
    : InlineTextBoxBase(other) {}

InlineTextBox::~InlineTextBox() {}

InlineBox* InlineTextBox::Copy() const {
  return new InlineTextBox(*this);
}

// Returns bounds rectangle of caret at |offset|. Caret is placed before
// character at |offset|. So, height of caret is height of character before
// |offset|.
gfx::RectF InlineTextBox::HitTestTextPosition(text::Offset offset) const {
  if (offset < start() || offset > end())
    return gfx::RectF();
  auto const length = static_cast<size_t>(offset - start());
  auto const left =
      length ? style().font().GetTextWidth(characters().data(), length) : 0.0f;
  return gfx::RectF(gfx::PointF(left, top()), gfx::SizeF(1.0f, height()));
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
                                   text::Offset lPosn,
                                   const base::string16& characters)
    : InlineTextBoxBase(style, width, height + 4.0f, lPosn, characters) {}

InlineUnicodeBox::InlineUnicodeBox(const InlineUnicodeBox& other)
    : InlineTextBoxBase(other) {}

InlineUnicodeBox::~InlineUnicodeBox() {}

// InlineBox
InlineBox* InlineUnicodeBox::Copy() const {
  return new InlineUnicodeBox(*this);
}

gfx::RectF InlineUnicodeBox::HitTestTextPosition(text::Offset offset) const {
  if (offset < start() || offset > end())
    return gfx::RectF();
  return gfx::RectF(gfx::PointF(width(), top()), gfx::SizeF(1.0f, height()));
}

bool InlineUnicodeBox::Merge(const RenderStyle&, float) {
  return false;
}

}  // namespace layout
