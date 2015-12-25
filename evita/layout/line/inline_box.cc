// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <string>

#include "evita/layout/line/inline_box.h"

#include "base/logging.h"
#include "evita/gfx/font.h"
#include "evita/gfx/rect_f.h"
#include "evita/layout/line/inline_box_visitor.h"
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
                     float left,
                     float width,
                     float height,
                     text::OffsetDelta start,
                     text::OffsetDelta end,
                     float descent)
    : descent_(descent),
      end_(end),
      height_(height),
      left_(left),
      start_(start),
      style_(style),
      width_(width) {
  DCHECK_GE(width, 1.0f);
  DCHECK_GE(height, 1.0f);
  // |InlineFillerBox| and |LineWrap| marker hold |start| == |end|.
  DCHECK_LE(start_, end_);
}

InlineBox::~InlineBox() {}

text::OffsetDelta InlineBox::HitTestPoint(float point_x) const {
  return point_x >= width() ? end() : start();
}

//////////////////////////////////////////////////////////////////////
//
// InlineFillerBox
//
InlineFillerBox::InlineFillerBox(const RenderStyle& style,
                                 float left,
                                 float width,
                                 float height,
                                 text::OffsetDelta start)
    : InlineBox(style,
                left,
                width,
                height,
                start,
                start + text::OffsetDelta(0),
                0.0f) {}

InlineFillerBox::~InlineFillerBox() {}

gfx::RectF InlineFillerBox::HitTestTextPosition(text::OffsetDelta offset,
                                                float baseline) const {
  return gfx::RectF();
}

//////////////////////////////////////////////////////////////////////
//
// WithFont
//
WithFont::WithFont(const gfx::Font& font) : font_(font) {}
WithFont::~WithFont() {}

//////////////////////////////////////////////////////////////////////
//
// InlineMarkerBox
//
InlineMarkerBox::InlineMarkerBox(const RenderStyle& style,
                                 float left,
                                 float width,
                                 float height,
                                 text::OffsetDelta start,
                                 text::OffsetDelta end,
                                 TextMarker marker_name)
    : InlineBox(style, left, width, height, start, end, style.font().descent()),
      WithFont(style.font()),
      marker_name_(marker_name) {}

InlineMarkerBox::~InlineMarkerBox() {}

// InlineBox
gfx::RectF InlineMarkerBox::HitTestTextPosition(text::OffsetDelta offset,
                                                float baseline) const {
  if (offset < start() || offset >= end())
    return gfx::RectF();
  return gfx::RectF(gfx::PointF(0.0f, baseline - ascent()),
                    gfx::SizeF(1.0f, height()));
}

//////////////////////////////////////////////////////////////////////
//
// InlineTextBoxBase
//
InlineTextBoxBase::InlineTextBoxBase(const RenderStyle& style,
                                     float left,
                                     float width,
                                     float height,
                                     text::OffsetDelta start,
                                     text::OffsetDelta end,
                                     const base::string16& characters)
    : InlineBox(style, left, width, height, start, end, style.font().descent()),
      WithFont(style.font()),
      characters_(characters) {}

InlineTextBoxBase::~InlineTextBoxBase() {}

// InlineBox
text::OffsetDelta InlineTextBoxBase::HitTestPoint(float x) const {
  if (x >= width())
    return end();
  for (auto k = 1u; k <= characters_.length(); ++k) {
    auto const cx = style().font().GetTextWidth(characters_.data(), k);
    if (x < cx)
      return start() + text::OffsetDelta(static_cast<int>(k - 1));
  }
  return end();
}

//////////////////////////////////////////////////////////////////////
//
// InlineTextBox
//
InlineTextBox::InlineTextBox(const RenderStyle& style,
                             float left,
                             float width,
                             float height,
                             text::OffsetDelta start,
                             const base::string16& characters)
    : InlineTextBoxBase(style,
                        left,
                        width,
                        height,
                        start,
                        start + text::OffsetDelta(characters.size()),
                        characters) {}

InlineTextBox::~InlineTextBox() {}

// Returns bounds rectangle of caret at |offset|. Caret is placed before
// character at |offset|. So, height of caret is height of character before
// |offset|.
gfx::RectF InlineTextBox::HitTestTextPosition(text::OffsetDelta offset,
                                              float baseline) const {
  if (offset < start() || offset > end())
    return gfx::RectF();
  const auto length = static_cast<size_t>(offset - start());
  const auto top = baseline - ascent();
  if (length == 0)
    return gfx::RectF(gfx::PointF(0.0f, top), gfx::SizeF(1.0f, height()));
  auto const width = font().GetTextWidth(characters().data(), length);
  return gfx::RectF(gfx::PointF(::ceil(width), top),
                    gfx::SizeF(1.0f, height()));
}

//////////////////////////////////////////////////////////////////////
//
// InlineUnicodeBox
//
InlineUnicodeBox::InlineUnicodeBox(const RenderStyle& style,
                                   float left,
                                   float width,
                                   float height,
                                   text::OffsetDelta start,
                                   const base::string16& characters)
    : InlineTextBoxBase(style,
                        left,
                        width,
                        height,
                        start,
                        start + text::OffsetDelta(1),
                        characters) {}

InlineUnicodeBox::~InlineUnicodeBox() {}

// InlineBox
text::OffsetDelta InlineUnicodeBox::HitTestPoint(float point_x) const {
  if (point_x >= width())
    return end();
  return start();
}

gfx::RectF InlineUnicodeBox::HitTestTextPosition(text::OffsetDelta offset,
                                                 float baseline) const {
  if (offset < start() || offset >= end())
    return gfx::RectF();
  return gfx::RectF(gfx::PointF(0.0f, baseline - ascent()),
                    gfx::SizeF(1.0f, height()));
}

}  // namespace layout
