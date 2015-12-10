// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cmath>

#include "evita/views/text/root_inline_box.h"

#include "base/logging.h"
#include "evita/paint/inline_box_painter.h"
#include "evita/views/text/inline_box.h"
#include "evita/views/text/render_selection.h"

namespace views {
namespace rendering {

RootInlineBox::RootInlineBox(const RootInlineBox& other)
    : m_nHash(other.m_nHash),
      m_lEnd(other.m_lEnd),
      m_lStart(other.m_lStart),
      bounds_(other.bounds_) {
  for (auto cell : other.cells_) {
    cells_.push_back(cell->Copy());
  }
}

RootInlineBox::RootInlineBox() : m_nHash(0), m_lStart(0), m_lEnd(0) {}

RootInlineBox::~RootInlineBox() {}

void RootInlineBox::set_origin(const gfx::PointF& origin) {
  bounds_.right = bounds_.width() + origin.x;
  bounds_.bottom = bounds_.height() + origin.y;
  bounds_.left = origin.x;
  bounds_.top = origin.y;
}

gfx::RectF RootInlineBox::CalculateSelectionRect(
    const TextSelection& selection) const {
  DCHECK(selection.is_range());
  if (selection.start() >= text_end() || selection.end() <= text_start())
    return gfx::RectF();
  auto const left =
      HitTestTextPosition(Contains(selection.start()) ? selection.start()
                                                      : text_start())
          .left;
  auto const right = Contains(selection.end())
                         ? HitTestTextPosition(selection.end()).left
                         : bounds_.right;
  return gfx::RectF(left, bounds_.top, right, bounds_.bottom);
}

bool RootInlineBox::Contains(text::Posn offset) const {
  return offset >= text_start() && offset < text_end();
}

RootInlineBox* RootInlineBox::Copy() const {
  return new RootInlineBox(*this);
}

bool RootInlineBox::Equal(const RootInlineBox* other) const {
  if (Hash() != other->Hash())
    return false;
  if (cells_.size() != other->cells_.size())
    return false;
  auto other_it = other->cells_.begin();
  for (auto cell : cells_) {
    if (!cell->Equal(*other_it))
      return false;
    ++other_it;
  }
  return true;
}

void RootInlineBox::AddInlineBox(InlineBox* cell) {
  cells_.push_back(cell);
}

void RootInlineBox::Fix(float ascent, float descent) {
  auto const left = 0.0f;
  auto const top = 0.0f;
  auto const height = ascent + descent;
  auto right = left;
  for (auto cell : cells_) {
    auto const lEnd = cell->Fix(height, descent);
    if (lEnd >= 0)
      m_lEnd = lEnd;
    right += cell->width();
  }
  bounds_.left = left;
  bounds_.top = top;
  bounds_.right = right;
  bounds_.bottom = top + height;
  DCHECK_EQ(bounds_.top, ::floor(bounds_.top));
  DCHECK_EQ(bounds_.bottom, ::floor(bounds_.bottom));
}

uint32_t RootInlineBox::Hash() const {
  if (m_nHash)
    return m_nHash;
  for (const auto cell : cells_) {
    m_nHash <<= 5;
    m_nHash ^= cell->Hash();
    m_nHash >>= 3;
  }
  return m_nHash;
}

gfx::RectF RootInlineBox::HitTestTextPosition(text::Posn offset) const {
  if (offset < m_lStart || offset >= m_lEnd)
    return gfx::RectF();

  auto origin = bounds_.origin();
  for (const auto cell : cells_) {
    auto const rect = cell->HitTestTextPosition(offset);
    if (!rect.empty())
      return gfx::RectF(origin + rect.origin(), rect.size());
    origin.x += cell->width();
  }

  return gfx::RectF();
}

bool RootInlineBox::IsEndOfDocument() const {
  auto const last_marker_cell = last_cell()->as<InlineMarkerBox>();
  return last_marker_cell->marker_name() == TextMarker::EndOfDocument;
}

text::Posn RootInlineBox::MapXToPosn(float xGoal) const {
  auto xInlineBox = 0.0f;
  auto lPosn = GetEnd() - 1;
  for (const auto cell : cells_) {
    auto const x = xGoal - xInlineBox;
    xInlineBox += cell->width();
    auto const lMap = cell->MapXToPosn(x);
    if (lMap >= 0)
      lPosn = lMap;
    if (x >= 0 && x < cell->width())
      break;
  }
  return lPosn;
}

void RootInlineBox::Render(gfx::Canvas* canvas) const {
  auto x = bounds_.left;
  for (auto cell : cells_) {
    gfx::RectF rect(x, bounds_.top, x + cell->width(),
                    ::ceil(bounds_.top + cell->line_height()));
    paint::InlineBoxPainter painter(canvas, rect);
    painter.Paint(*cell);
    x = rect.right;
  }
}

}  // namespace rendering
}  // namespace views
