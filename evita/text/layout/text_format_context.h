// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_LAYOUT_TEXT_FORMAT_CONTEXT_H_
#define EVITA_TEXT_LAYOUT_TEXT_FORMAT_CONTEXT_H_

#include "evita/gfx/rect_f.h"
#include "evita/text/models/offset.h"

namespace text {
class Buffer;
class MarkerSet;
}

namespace layout {

class StyleTree;

class TextFormatContext {
 public:
  TextFormatContext(const text::Buffer& buffer,
                    const text::MarkerSet& markers,
                    const StyleTree& style_tree,
                    text::Offset line_start,
                    text::Offset offset,
                    const gfx::RectF& bounds,
                    float zoom);
  TextFormatContext(const TextFormatContext& other);
  ~TextFormatContext();

  TextFormatContext& operator=(const TextFormatContext& other) = delete;

  const gfx::RectF& bounds() const { return bounds_; }
  const text::Buffer& buffer() const { return buffer_; }
  const text::Offset line_start() const { return line_start_; }
  const text::MarkerSet& markers() const { return markers_; }
  const text::Offset offset() const { return offset_; }
  const StyleTree& style_tree() const { return style_tree_; }
  float zoom() const { return zoom_; }

 private:
  const gfx::RectF& bounds_;
  const text::Buffer& buffer_;
  const text::Offset line_start_;
  const text::MarkerSet& markers_;
  const text::Offset offset_;
  const StyleTree& style_tree_;
  const float zoom_;
};

}  // namespace layout

#endif  // EVITA_TEXT_LAYOUT_TEXT_FORMAT_CONTEXT_H_
