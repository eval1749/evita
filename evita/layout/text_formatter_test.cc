// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <cmath>
#include <numeric>

#include "evita/gfx/font.h"
#include "evita/gfx/rect_conversions.h"
#include "evita/layout/line/inline_box.h"
#include "evita/layout/line/root_inline_box.h"
#include "evita/layout/text_formatter.h"
#include "evita/layout/render_font_set.h"
#include "evita/text/buffer.h"
#include "gtest/gtest.h"

namespace layout {

//////////////////////////////////////////////////////////////////////
//
// TextFormatterTest
//
class TextFormatterTest : public ::testing::Test {
 protected:
  TextFormatterTest();
  ~TextFormatterTest() override = default;

  text::Buffer* buffer() const { return buffer_.get(); }

  RenderStyle StyleAt(text::Offset offset) const;
  float WidthOf(const RenderStyle& style, const base::string16& text) const;

 private:
  std::unique_ptr<text::Buffer> buffer_;

  DISALLOW_COPY_AND_ASSIGN(TextFormatterTest);
};

TextFormatterTest::TextFormatterTest() : buffer_(new text::Buffer()) {}

RenderStyle TextFormatterTest::StyleAt(text::Offset offset) const {
  auto style = buffer()->GetStyleAt(offset);
  style.Merge(buffer()->GetDefaultStyle());
  const auto sample =
      offset < buffer()->GetEnd() ? buffer()->GetCharAt(offset) : 'x';
  const auto font = FontSet::GetFont(style, sample);
  return RenderStyle(style, *font);
}

float TextFormatterTest::WidthOf(const RenderStyle& style,
                                 const base::string16& text) const {
  return ::ceil(style.font().GetTextWidth(text));
}

TEST_F(TextFormatterTest, FormatLineBasic) {
  buffer()->InsertBefore(text::Offset(0), L"foo");
  const auto& origin = gfx::PointF(300.0f, 200.0f);
  const auto& bounds = gfx::RectF(origin, gfx::SizeF(100.0f, 50.0f));

  TextFormatter formatter1(*buffer(), text::Offset(0), text::Offset(0), bounds,
                           1.0f);
  const auto line1 = formatter1.FormatLine();
  line1->set_origin(origin);
  const auto height = line1->height();

  EXPECT_EQ(3, line1->boxes().size());
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
  EXPECT_EQ(gfx::RectF(origin, gfx::SizeF(boxes_width, boxes_height)),
            line1->bounds());
  EXPECT_EQ(
      gfx::RectF(gfx::PointF(origin.x + line1->boxes()[0]->width() +
                                 WidthOf(line1->boxes()[1]->style(), L"f"),
                             origin.y),
                 gfx::SizeF(1.0f, line1->boxes()[1]->height())),
      line1->HitTestTextPosition(text::Offset(1)));

  // Change background color offset 1 and format again
  css::Style style;
  style.set_bgcolor(css::Color(255, 0, 0));
  buffer()->SetStyle(text::Offset(1), text::Offset(2), style);
  TextFormatter formatter2(*buffer(), text::Offset(0), text::Offset(0), bounds,
                           1.0f);
  const auto line2 = formatter2.FormatLine();
  line2->set_origin(origin);
  EXPECT_EQ(5, line2->boxes().size());
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

TEST_F(TextFormatterTest, FormatLineMissingCharacter) {
  buffer()->InsertBefore(text::Offset(0), L"f\uFFFFo");
  const auto& origin = gfx::PointF(10.0f, 200.0f);
  const auto& bounds = gfx::RectF(origin, gfx::SizeF(100.0f, 50.0f));

  TextFormatter formatter1(*buffer(), text::Offset(0), text::Offset(0), bounds,
                           1.0f);
  const auto line1 = formatter1.FormatLine();
  line1->set_origin(origin);
  EXPECT_EQ(5, line1->boxes().size()) << "line width is " << line1->width();
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
  EXPECT_EQ(gfx::RectF(origin, gfx::SizeF(boxes_width, boxes_height)),
            line1->bounds());
}

TEST_F(TextFormatterTest, FormatLineWrap) {
  buffer()->InsertBefore(text::Offset(0), L"0123456");
  const auto& bounds = gfx::RectF(gfx::SizeF(40.0f, 50.0f));
  TextFormatter formatter1(*buffer(), text::Offset(0), text::Offset(0), bounds,
                           1.0f);
  // View:
  //    012>
  //    345>
  //    6<
  const auto line1 = formatter1.FormatLine();
  EXPECT_EQ(TextMarker::LineWrap,
            line1->boxes().back()->as<InlineMarkerBox>()->marker_name());
  EXPECT_FALSE(line1->IsContinuedLine());
  EXPECT_EQ(0, line1->line_start());
  EXPECT_EQ(0, line1->text_start());
  EXPECT_EQ(3, line1->text_end());

  const auto line2 = formatter1.FormatLine();
  EXPECT_EQ(line1->line_start(), line2->line_start());
  EXPECT_EQ(3, line2->text_start());
  EXPECT_EQ(6, line2->text_end());
  EXPECT_TRUE(line2->IsContinuedLine());

  const auto line3 = formatter1.FormatLine();
  EXPECT_EQ(line1->line_start(), line3->line_start());
  EXPECT_EQ(6, line3->text_start());
  EXPECT_EQ(8, line3->text_end()) << "document.length + 1";
  EXPECT_TRUE(line3->IsContinuedLine());
}
}  // namespace layout
