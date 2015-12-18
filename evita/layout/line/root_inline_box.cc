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
                             text::Offset text_start,
                             text::Offset text_end,
                             float ascent,
                             float descent)
    : boxes_(boxes),
      descent_(descent),
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

gfx::RectF RootInlineBox::HitTestTextPosition(text::Offset offset) const {
  DCHECK(offset.IsValid());
  DCHECK(!boxes_.empty());
  if (offset < text_start_ || offset >= text_end_)
    return gfx::RectF();

  const auto baseline = height() - descent_;
  auto origin = bounds_.origin();
  for (const auto& box : boxes_) {
    const auto rect = box->HitTestTextPosition(offset, baseline);
    if (!rect.empty()) {
      return gfx::RectF(gfx::ToEnclosingRect(
          gfx::RectF(origin + rect.origin(), rect.size())));
    }
    origin.x += box->width();
  }

  return gfx::RectF();
}

bool RootInlineBox::IsEndOfDocument() const {
  DCHECK(!boxes_.empty());
  const auto last_marker_box = last_box()->as<InlineMarkerBox>();
  return last_marker_box->marker_name() == TextMarker::EndOfDocument;
}

// Map x-coordinate in content space to text offset.
text::Offset RootInlineBox::MapXToPosn(float point_x) const {
  if (point_x <= boxes_[1]->left())
    return text_start();
  const auto& it = std::lower_bound(
      std::next(boxes_.begin()), boxes_.end(), point_x,
      [](const InlineBox* box1, float value) { return box1->left() < value; });
  if (it == boxes_.end())
    return text_end() - text::OffsetDelta(1);
  const auto box = (*it)->left() == point_x ? *it : *std::prev(it);
  const auto offset = box->MapXToPosn(point_x - box->left());
  if (!offset.IsValid())
    return text_end() - text::OffsetDelta(1);
  return offset;
}

}  // namespace layout
