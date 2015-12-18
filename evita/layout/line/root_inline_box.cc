// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cmath>

#include "evita/layout/line/root_inline_box.h"

#include "base/logging.h"
#include "evita/paint/inline_box_painter.h"
#include "evita/layout/line/inline_box.h"
#include "evita/layout/render_selection.h"

namespace layout {

namespace {
std::vector<InlineBox*> CopyInlineBoxList(const std::vector<InlineBox*> boxes) {
  std::vector<InlineBox*> copy;
  copy.reserve(boxes.size());
  for (auto const& box : boxes)
    copy.push_back(box->Copy());
  return std::move(copy);
}
}  // namespace

RootInlineBox::RootInlineBox(const std::vector<InlineBox*>& boxes,
                             text::Offset text_start,
                             text::Offset text_end,
                             float ascent,
                             float descent)
    : boxes_(boxes),
      descent_(descent),
      m_nHash(0),
      text_start_(text_start),
      text_end_(text_end) {
  DCHECK(!boxes_.empty());
  auto const left = 0.0f;
  auto const top = 0.0f;
  auto const height = ascent + descent;
  auto right = left;
  for (auto box : boxes_) {
    auto const text_end = box->Fix(height, descent);
    if (text_end >= 0)
      text_end_ = text_end;
    right += box->width();
  }
  bounds_.left = left;
  bounds_.top = top;
  bounds_.right = right;
  bounds_.bottom = top + height;
  DCHECK_EQ(bounds_.top, ::floor(bounds_.top));
  DCHECK_EQ(bounds_.bottom, ::floor(bounds_.bottom));
}

RootInlineBox::RootInlineBox(const RootInlineBox& other)
    : boxes_(CopyInlineBoxList(other.boxes_)),
      m_nHash(other.m_nHash),
      text_end_(other.text_end_),
      text_start_(other.text_start_),
      bounds_(other.bounds_) {}

RootInlineBox::~RootInlineBox() {}

void RootInlineBox::set_origin(const gfx::PointF& origin) {
  bounds_.right = bounds_.width() + origin.x;
  bounds_.bottom = bounds_.height() + origin.y;
  bounds_.left = origin.x;
  bounds_.top = origin.y;
}

bool RootInlineBox::Contains(text::Offset offset) const {
  DCHECK(offset.IsValid());
  DCHECK(!boxes_.empty());
  return offset >= text_start() && offset < text_end();
}

RootInlineBox* RootInlineBox::Copy() const {
  DCHECK(!boxes_.empty());
  return new RootInlineBox(*this);
}

bool RootInlineBox::Equal(const RootInlineBox* other) const {
  DCHECK(!boxes_.empty());
  if (Hash() != other->Hash())
    return false;
  if (boxes_.size() != other->boxes_.size())
    return false;
  auto other_it = other->boxes_.begin();
  for (auto box : boxes_) {
    if (!box->Equal(*other_it))
      return false;
    ++other_it;
  }
  return true;
}

uint32_t RootInlineBox::Hash() const {
  DCHECK(!boxes_.empty());
  if (m_nHash)
    return m_nHash;
  for (const auto box : boxes_) {
    m_nHash <<= 5;
    m_nHash ^= box->Hash();
    m_nHash >>= 3;
  }
  return m_nHash;
}

gfx::RectF RootInlineBox::HitTestTextPosition(text::Offset offset) const {
  DCHECK(offset.IsValid());
  DCHECK(!boxes_.empty());
  if (offset < text_start_ || offset >= text_end_)
    return gfx::RectF();

  auto origin = bounds_.origin();
  for (const auto box : boxes_) {
    auto const rect = box->HitTestTextPosition(offset);
    if (!rect.empty())
      return gfx::RectF(origin + rect.origin(), rect.size());
    origin.x += box->width();
  }

  return gfx::RectF();
}

bool RootInlineBox::IsEndOfDocument() const {
  DCHECK(!boxes_.empty());
  auto const last_marker_box = last_box()->as<InlineMarkerBox>();
  return last_marker_box->marker_name() == TextMarker::EndOfDocument;
}

text::Offset RootInlineBox::MapXToPosn(float xGoal) const {
  DCHECK(!boxes_.empty());
  auto xInlineBox = 0.0f;
  auto offset = text_end() - text::OffsetDelta(1);
  for (const auto box : boxes_) {
    auto const x = xGoal - xInlineBox;
    xInlineBox += box->width();
    auto const lMap = box->MapXToPosn(x);
    if (lMap >= 0)
      offset = lMap;
    if (x >= 0 && x < box->width())
      break;
  }
  return offset;
}

}  // namespace layout
