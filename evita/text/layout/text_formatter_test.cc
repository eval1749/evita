// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <cmath>
#include <numeric>

#include "evita/text/layout/text_layout_test_base.h"

#include "evita/base/strings/atomic_string.h"
#include "evita/gfx/font.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/text/layout/line/inline_box.h"
#include "evita/text/layout/line/root_inline_box.h"
#include "evita/text/layout/text_format_context.h"
#include "evita/text/layout/text_formatter.h"
#include "evita/text/models/buffer.h"
#include "evita/text/models/marker_set.h"
#include "evita/text/models/static_range.h"
#include "evita/text/style/computed_style_builder.h"

namespace layout {

//////////////////////////////////////////////////////////////////////
//
// TextFormatterTest
//
class TextFormatterTest : public TextLayoutTestBase {
 protected:
  TextFormatterTest() = default;
  ~TextFormatterTest() override = default;

  float WidthOf(const ComputedStyle& style, const base::string16& text) const;

 private:
  DISALLOW_COPY_AND_ASSIGN(TextFormatterTest);
};

float TextFormatterTest::WidthOf(const ComputedStyle& style,
                                 const base::string16& text) const {
  return ::ceil(style.fonts()[0]->GetTextWidth(text));
}

TEST_F(TextFormatterTest, FormatLineBasic) {
  buffer()->InsertBefore(text::Offset(0), L"foo");
  TextFormatter formatter1(FormatContextFor(text::Offset(0)));
  const auto line1 = formatter1.FormatLine();
  line1->set_origin(origin());
  const auto height = line1->height();

  ASSERT_EQ(3, line1->boxes().size());
  EXPECT_TRUE(line1->boxes()[0]->is<InlineFillerBox>());
  EXPECT_TRUE(line1->boxes()[1]->is<InlineTextBox>());
  EXPECT_TRUE(line1->boxes()[2]->is<InlineMarkerBox>());
  const auto boxes_width = std::accumulate(
      line1->boxes().begin(), line1->boxes().end(), 0.0f,
      [](float sum, InlineBox* box) { return sum + box->width(); });
  const auto boxes_ascent = std::accumulate(
      line1->boxes().begin(), line1->boxes().end(), 0.0f,
      [](float max, InlineBox* box) { return std::max(max, box->ascent()); });
  const auto boxes_descent = std::accumulate(
      line1->boxes().begin(), line1->boxes().end(), 0.0f,
      [](float max, InlineBox* box) { return std::max(max, box->descent()); });
  const auto boxes_height = boxes_ascent + boxes_descent;
  EXPECT_EQ(gfx::RectF(origin(), gfx::SizeF(boxes_width, boxes_height)),
            line1->bounds());
  EXPECT_EQ(
      gfx::RectF(gfx::PointF(origin().x + line1->boxes()[0]->width() +
                                 WidthOf(line1->boxes()[1]->style(), L"f"),
                             origin().y),
                 gfx::SizeF(1.0f, line1->boxes()[1]->height())),
      line1->HitTestTextPosition(text::Offset(1)));

  // Change color offset 1 and format again
  buffer()->syntax_markers()->InsertMarker(
      text::StaticRange(*buffer(), text::Offset(1), text::Offset(2)),
      base::AtomicString(L"keyword"));
  TextFormatter formatter2(FormatContextFor(text::Offset(0)));
  const auto line2 = formatter2.FormatLine();
  line2->set_origin(origin());
  ASSERT_EQ(5, line2->boxes().size());
  EXPECT_TRUE(line2->boxes()[0]->is<InlineFillerBox>());
  EXPECT_TRUE(line2->boxes()[1]->is<InlineTextBox>());
  EXPECT_TRUE(line2->boxes()[2]->is<InlineTextBox>());
  EXPECT_TRUE(line2->boxes()[3]->is<InlineTextBox>());
  EXPECT_TRUE(line2->boxes()[4]->is<InlineMarkerBox>());
  // TODO(eval1749): Once RootInlineBox should have integer bounds, we should
  // not use |gfx::ToEnclosingRect()|.
  EXPECT_EQ(gfx::ToEnclosingRect(line1->bounds()),
            gfx::ToEnclosingRect(line2->bounds()))
      << "Changing background color doesn't change line bounds.";
}

TEST_F(TextFormatterTest, FormatLineMarker) {
  buffer()->InsertBefore(text::Offset(0), L"<abc>");
  // Set marker to "abc".
  markers()->InsertMarker(
      text::StaticRange(*buffer(), text::Offset(0), text::Offset(1)),
      base::AtomicString(L"keyword"));
  markers()->InsertMarker(
      text::StaticRange(*buffer(), text::Offset(4), text::Offset(5)),
      base::AtomicString(L"keyword"));
  TextFormatter formatter1(FormatContextFor(text::Offset(0)));
  const auto line1 = formatter1.FormatLine();
  EXPECT_EQ(5, line1->boxes().size());
  EXPECT_EQ(text::OffsetDelta(1), line1->boxes()[2]->start())
      << "'abc' should be paint differently";
  EXPECT_EQ(text::OffsetDelta(4), line1->boxes()[2]->end())
      << "'abc' should be paint differently";
}

TEST_F(TextFormatterTest, FormatLineMissingCharacter) {
  buffer()->InsertBefore(text::Offset(0), L"f\uFFFFo");
  TextFormatter formatter1(FormatContextFor(text::Offset(0)));
  const auto line1 = formatter1.FormatLine();
  line1->set_origin(origin());
  ASSERT_EQ(5, line1->boxes().size()) << "line width is " << line1->width();
  EXPECT_TRUE(line1->boxes()[0]->is<InlineFillerBox>());
  EXPECT_TRUE(line1->boxes()[1]->is<InlineTextBox>());
  EXPECT_TRUE(line1->boxes()[2]->is<InlineUnicodeBox>());
  EXPECT_TRUE(line1->boxes()[3]->is<InlineTextBox>());
  EXPECT_TRUE(line1->boxes()[4]->is<InlineMarkerBox>());
  const auto boxes_width = std::accumulate(
      line1->boxes().begin(), line1->boxes().end(), 0.0f,
      [](float sum, InlineBox* box) { return sum + box->width(); });
  const auto boxes_ascent = std::accumulate(
      line1->boxes().begin(), line1->boxes().end(), 0.0f,
      [](float max, InlineBox* box) { return std::max(max, box->ascent()); });
  const auto boxes_descent = std::accumulate(
      line1->boxes().begin(), line1->boxes().end(), 0.0f,
      [](float max, InlineBox* box) { return std::max(max, box->descent()); });
  const auto boxes_height = boxes_ascent + boxes_descent;
  EXPECT_EQ(gfx::RectF(origin(), gfx::SizeF(boxes_width, boxes_height)),
            line1->bounds());
}

TEST_F(TextFormatterTest, FormatLineWrap) {
  buffer()->InsertBefore(text::Offset(0), L"0123456");
  set_bounds(gfx::RectF(gfx::SizeF(40.0f, 50.0f)));
  TextFormatter formatter1(FormatContextFor(text::Offset(0)));
  // View:
  //    012>
  //    345>
  //    6<
  const auto line1 = formatter1.FormatLine();
  EXPECT_EQ(TextMarker::LineWrap,
            line1->boxes().back()->as<InlineMarkerBox>()->marker_name());
  EXPECT_FALSE(line1->IsContinuedLine());
  EXPECT_EQ(text::Offset(0), line1->line_start());
  EXPECT_EQ(text::Offset(0), line1->text_start());
  EXPECT_EQ(text::Offset(3), line1->text_end());

  const auto line2 = formatter1.FormatLine();
  EXPECT_EQ(line1->line_start(), line2->line_start());
  EXPECT_EQ(text::Offset(3), line2->text_start());
  EXPECT_EQ(text::Offset(6), line2->text_end());
  EXPECT_TRUE(line2->IsContinuedLine());

  const auto line3 = formatter1.FormatLine();
  EXPECT_EQ(line1->line_start(), line3->line_start());
  EXPECT_EQ(text::Offset(6), line3->text_start());
  EXPECT_EQ(text::Offset(8), line3->text_end()) << "document.length + 1";
  EXPECT_TRUE(line3->IsContinuedLine());
}
}  // namespace layout
