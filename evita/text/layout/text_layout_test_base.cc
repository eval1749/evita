// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/layout/text_layout_test_base.h"

#include "evita/text/layout/text_format_context.h"
#include "evita/text/models/buffer.h"
#include "evita/text/models/marker_set.h"

namespace layout {

TextLayoutTestBase::TextLayoutTestBase()
    : bounds_(gfx::PointF(300, 200), gfx::SizeF(100, 50)),
      buffer_(new text::Buffer()),
      markers_(new text::MarkerSet(*buffer_)),
      zoom_(1.0f) {}

TextFormatContext TextLayoutTestBase::FormatContextFor(
    text::Offset line_start,
    text::Offset offset) const {
  return TextFormatContext(*buffer_, *markers_, line_start, offset, bounds_,
                           zoom_);
}

TextFormatContext TextLayoutTestBase::FormatContextFor(
    text::Offset offset) const {
  const auto line_start = buffer_->ComputeStartOfLine(offset);
  return FormatContextFor(line_start, offset);
}

TextLayoutTestBase::~TextLayoutTestBase() = default;

}  // namespace layout
