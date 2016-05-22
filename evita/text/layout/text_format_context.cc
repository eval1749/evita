// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/text/layout/text_format_context.h"

#include "evita/text/models/buffer.h"

namespace layout {

TextFormatContext::TextFormatContext(const text::Buffer& buffer,
                    const text::MarkerSet& markers,
                    text::Offset line_start,
                    text::Offset offset,
                    const gfx::RectF& bounds,
                    float zoom)
    : buffer_(buffer),
      bounds_(bounds),
      line_start_(line_start),
      markers_(markers),
      offset_(offset),
      zoom_(zoom) {
  DCHECK_EQ(line_start_, buffer_.ComputeStartOfLine(offset_));
}

TextFormatContext::TextFormatContext(const TextFormatContext& other)
    : TextFormatContext(other.buffer_, other.markers_, other.line_start_,
                        other.offset_, other.bounds_, other.zoom_) {}

TextFormatContext::~TextFormatContext() = default;

}  // namespace layout
