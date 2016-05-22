// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_TEXT_LAYOUT_TEXT_LAYOUT_TEST_BASE_H_
#define EVITA_TEXT_LAYOUT_TEXT_LAYOUT_TEST_BASE_H_

#include <memory>

#include "evita/gfx/rect_f.h"
#include "evita/text/models/offset.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace css {
class StyleSheet;
}

namespace text {
class Buffer;
class MarkerSet;
}

namespace layout {

class TextFormatContext;
class StyleTree;

class TextLayoutTestBase : public ::testing::Test {
 protected:
  TextLayoutTestBase(const TextLayoutTestBase& other) = delete;
  TextLayoutTestBase();
  ~TextLayoutTestBase() override;

  TextLayoutTestBase& operator=(const TextLayoutTestBase& other) = delete;

  const gfx::RectF& bounds() const { return bounds_; }
  void set_bounds(const gfx::RectF& bounds) { bounds_ = bounds; }
  text::Buffer* buffer() const { return buffer_.get(); }
  text::MarkerSet* markers() const { return markers_.get(); }
  gfx::PointF origin() const { return bounds_.origin(); }
  css::StyleSheet* style_sheet() const { return style_sheet_; }
  const StyleTree& style_tree() const { return *style_tree_; }
  float zoom() const { return zoom_; }
  void set_zoom(float zoom) { zoom_ = zoom; }

  TextFormatContext FormatContextFor(text::Offset line_start,
                                     text::Offset offset) const;
  TextFormatContext FormatContextFor(text::Offset offset) const;

 private:
  gfx::RectF bounds_;
  const std::unique_ptr<text::Buffer> buffer_;
  const std::unique_ptr<text::MarkerSet> markers_;
  css::StyleSheet* style_sheet_;
  const std::unique_ptr<StyleTree> style_tree_;
  float zoom_ = 1.0f;
};

}  // namespace layout

#endif  // EVITA_TEXT_LAYOUT_TEXT_LAYOUT_TEST_BASE_H_
