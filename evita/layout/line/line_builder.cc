// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/layout/line/line_builder.h"

#include "base/logging.h"
#include "evita/layout/line/inline_box.h"
#include "evita/layout/line/root_inline_box.h"

namespace layout {

namespace {

// TODO(eval1749): We should move |AlignHeightToPixel()| to another place
// to share code.
float AlignHeightToPixel(float height) {
  return gfx::FactorySet::instance()
      ->AlignToPixel(gfx::SizeF(0.0f, height))
      .height;
}

}  // namespace

LineBuilder::LineBuilder(text::Offset text_start)
    : ascent_(0),
      current_x_(0),
      descent_(0),
      last_box_width_(0),
      text_start_(text_start) {}

LineBuilder::~LineBuilder() {}

void LineBuilder::Add(InlineBox* box) {
  if (boxes_.empty() || boxes_.back() != box)
    boxes_.push_back(box);
  else
    current_x_ -= last_box_width_;
  current_x_ += box->width();
  ascent_ = std::max(box->height() - box->descent(), ascent_);
  descent_ = std::max(box->descent(), descent_);
  last_box_width_ = box->width();
}

// We have at least one cell.
// o end of buffer: End-Of-Buffer InlineMarkerBox
// o end of line: End-Of-Line InlineMarkerBox
// o wrapped line: Warp InlineMarkerBox
scoped_refptr<RootInlineBox> LineBuilder::Build(text::Offset text_end) {
  DCHECK(!boxes_.empty());
  return make_scoped_refptr(new RootInlineBox(boxes_, text_start_, text_end,
                                              AlignHeightToPixel(ascent_),
                                              AlignHeightToPixel(descent_)));
}

}  // namespace layout
