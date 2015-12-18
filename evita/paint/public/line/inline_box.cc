// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <cmath>
#include <string>

#include "evita/paint/public/line/inline_box.h"

#include "base/logging.h"
#include "evita/layout/render_font.h"
#include "evita/layout/render_style.h"
#include "evita/paint/public/line/inline_box_visitor.h"

namespace paint {

#define V(name) \
  void name::Accept(InlineBoxVisitor* visitor) { visitor->Visit##name(this); }
FOR_EACH_PAINT_INLINE_BOX(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// InlineBox
//
InlineBox::InlineBox(const RenderStyle& style,
                     float width,
                     float height,
                     float descent,
                     float line_height,
                     float line_descent)
    : descent_(descent),
      height_(height),
      line_descent_(line_descent),
      line_height_(line_height),
      style_(style),
      width_(width) {
  DCHECK_GE(width, 1.0f);
  DCHECK_GE(height, 1.0f);
}

InlineBox::~InlineBox() {}

float InlineBox::top() const {
  // TODO(eval1749): Once |layout::InlineBox| does floor, we don't need to do
  // call |floor()| here.
  return ::floor(line_height() - line_descent() - height() + descent());
}

// InlineBox
bool InlineBox::Equal(const InlineBox* other) const {
  return other->class_name() == class_name() && other->width_ == width_ &&
         other->line_height_ == line_height_ && other->style_ == style_;
}

size_t InlineBox::Hash() const {
  auto hash_code = static_cast<size_t>(width_);
  hash_code ^= static_cast<size_t>(line_height_);
  hash_code ^= std::hash<RenderStyle>()(style_);
  return hash_code;
}

//////////////////////////////////////////////////////////////////////
//
// InlineFillerBox
//
InlineFillerBox::InlineFillerBox(const RenderStyle& style,
                                 float width,
                                 float height,
                                 float line_height,
                                 float line_descent)
    : InlineBox(style, width, height, 0.0f, line_height, line_descent) {}

InlineFillerBox::~InlineFillerBox() {}

InlineBox* InlineFillerBox::Copy() const {
  return new InlineFillerBox(style(), width(), height(), line_height(),
                             line_descent());
}

//////////////////////////////////////////////////////////////////////
//
// WithFont
//
WithFont::WithFont(const Font& font) : font_(font) {}

WithFont::WithFont(const WithFont& other) : font_(other.font_) {}

WithFont::~WithFont() {}

//////////////////////////////////////////////////////////////////////
//
// InlineMarkerBox
//
InlineMarkerBox::InlineMarkerBox(const RenderStyle& style,
                                 float width,
                                 float height,
                                 TextMarker marker_name,
                                 float line_height,
                                 float line_descent)
    : InlineBox(style,
                width,
                height,
                style.font().descent(),
                line_height,
                line_descent),
      WithFont(style.font()),
      marker_name_(marker_name) {}

InlineMarkerBox::~InlineMarkerBox() {}

// InlineBox
InlineBox* InlineMarkerBox::Copy() const {
  return new InlineMarkerBox(style(), width(), height(), marker_name_,
                             line_height(), line_descent());
}

bool InlineMarkerBox::Equal(const InlineBox* other) const {
  if (!InlineBox::Equal(other))
    return false;
  auto const marker_cell = other->as<InlineMarkerBox>();
  return marker_name_ == marker_cell->marker_name_;
}

size_t InlineMarkerBox::Hash() const {
  auto hash_code = InlineBox::Hash();
  hash_code <<= 5;
  hash_code |= static_cast<size_t>(marker_name_);
  hash_code >>= 3;
  return hash_code;
}

//////////////////////////////////////////////////////////////////////
//
// InlineTextBoxBase
//
InlineTextBoxBase::InlineTextBoxBase(const RenderStyle& style,
                                     float width,
                                     float height,
                                     const base::string16& characters,
                                     float line_height,
                                     float line_descent)
    : InlineBox(style,
                width,
                height,
                style.font().descent(),
                line_height,
                line_descent),
      WithFont(style.font()),
      characters_(characters) {}

InlineTextBoxBase::~InlineTextBoxBase() {}

// InlineBox
bool InlineTextBoxBase::Equal(const InlineBox* other) const {
  if (!InlineBox::Equal(other))
    return false;
  return characters_ == other->as<InlineTextBoxBase>()->characters_;
}

size_t InlineTextBoxBase::Hash() const {
  return static_cast<size_t>((InlineBox::Hash() << 3) ^
                             std::hash<base::string16>()(characters_));
}

//////////////////////////////////////////////////////////////////////
//
// InlineTextBox
//
InlineTextBox::InlineTextBox(const RenderStyle& style,
                             float width,
                             float height,
                             const base::string16& characters,
                             float line_height,
                             float line_descent)
    : InlineTextBoxBase(style,
                        width,
                        height,
                        characters,
                        line_height,
                        line_descent) {}

InlineTextBox::~InlineTextBox() {}

InlineBox* InlineTextBox::Copy() const {
  return new InlineTextBox(style(), width(), height(), characters(),
                           line_height(), line_descent());
}

//////////////////////////////////////////////////////////////////////
//
// InlineUnicodeBox
//
InlineUnicodeBox::InlineUnicodeBox(const RenderStyle& style,
                                   float width,
                                   float height,
                                   const base::string16& characters,
                                   float line_height,
                                   float line_descent)
    : InlineTextBoxBase(style,
                        width,
                        height,
                        characters,
                        line_height,
                        line_descent) {}

InlineUnicodeBox::~InlineUnicodeBox() {}

// InlineBox
InlineBox* InlineUnicodeBox::Copy() const {
  return new InlineUnicodeBox(style(), width(), height(), characters(),
                              line_height(), line_descent());
}

}  // namespace paint
