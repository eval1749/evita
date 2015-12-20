// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <cmath>
#include <iterator>

#include "evita/layout/line/root_inline_box.h"

#include "base/logging.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/paint/inline_box_painter.h"
#include "evita/layout/line/inline_box.h"
#include "evita/layout/render_selection.h"

namespace layout {

//////////////////////////////////////////////////////////////////////
//
// RootInlineBox
//
RootInlineBox::RootInlineBox(const std::vector<InlineBox*>& boxes,
                             text::Offset line_start,
                             text::Offset text_start,
                             text::Offset text_end,
                             float ascent,
                             float descent)
    : boxes_(boxes),
      descent_(descent),
      line_start_(line_start),
      text_start_(text_start),
      text_end_(text_end) {
  DCHECK(!boxes_.empty());
  auto right = 0.0f;
  for (const auto& box : boxes_)
    right += box->width();
  bounds_.right = right;
  bounds_.bottom = ascent + descent;
  DCHECK_EQ(bounds_.top, ::floor(bounds_.top));
  DCHECK_EQ(bounds_.bottom, ::floor(bounds_.bottom));
}

RootInlineBox::~RootInlineBox() {
  for (const auto& box : boxes_)
    delete box;
}

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

// Map x-coordinate in content space to text offset.
text::Offset RootInlineBox::HitTestPoint(float point_x) const {
  DCHECK(boxes_.back()->is<InlineMarkerBox>());
  // Get box after |point_x|
  const auto& it = std::lower_bound(
      boxes_.begin(), boxes_.end(), point_x,
      [](const InlineBox* box, float value) { return box->left() < value; });
  if (it == boxes_.end())
    return text_end() - text::OffsetDelta(1);
  const auto box = it == boxes_.begin() ? *it : *std::prev(it);
  return text_start_ + box->HitTestPoint(point_x - box->left());
}

gfx::RectF RootInlineBox::HitTestTextPosition(text::Offset offset) const {
  DCHECK(offset.IsValid());
  DCHECK(!boxes_.empty());
  if (offset < text_start_ || offset >= text_end_)
    return gfx::RectF();
  const auto offset_in_line = offset - text_start_;
  // Search box after filler box
  const auto& first = std::next(boxes_.begin());
  const auto& it =
      std::lower_bound(first, boxes_.end(), offset_in_line,
                       [](const InlineBox* box, text::OffsetDelta value) {
                         return box->start() < value;
                       });
  if (it == boxes_.end())
    return gfx::RectF();
  const auto baseline = bounds_.height() - descent_;
  const auto box =
      it == first || (*it)->start() == offset_in_line ? *it : *std::prev(it);
  const auto& rect = box->HitTestTextPosition(offset_in_line, baseline);
  if (rect.empty())
    return rect;
  const auto box_origin = origin() + gfx::SizeF(box->left(), 0.0f);
  const auto caret_origin = rect.origin() + box_origin;
  return gfx::RectF(
      gfx::PointF(::floor(caret_origin.x), ::floor(caret_origin.y)),
      rect.size());
}

bool RootInlineBox::IsEndOfDocument() const {
  DCHECK(!boxes_.empty());
  const auto last_marker_box = last_box()->as<InlineMarkerBox>();
  return last_marker_box->marker_name() == TextMarker::EndOfDocument;
}

void RootInlineBox::UpdateTextStart(text::OffsetDelta delta) {
  text_start_ += delta;
  text_end_ += delta;
}

}  // namespace layout
