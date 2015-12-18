// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cmath>
#include <vector>

#include "evita/css/style.h"
#include "evita/layout/line/inline_box.h"
#include "evita/layout/line/root_inline_box.h"
#include "evita/layout/render_font.h"
#include "evita/layout/render_font_set.h"
#include "gtest/gtest.h"

namespace layout {

namespace {

//////////////////////////////////////////////////////////////////////
//
// LineBuilder
//
class LineBuilder final {
 public:
  LineBuilder(text::Offset start, const RenderStyle& style);
  ~LineBuilder() = default;

  float ascent() const { return style_.font().height() - descent(); }
  const std::vector<InlineBox*> boxes() const { return boxes_; }
  float descent() const { return style_.font().descent(); }
  text::Offset text_end() const { return offset_; }
  text::Offset text_start() const { return start_; }

  void AddCodeUnit(const base::char16 code_unit);
  void AddMarker(TextMarker marker);
  void AddText(const base::string16& text);
  float WidthOf(const base::string16& text) const;

 private:
  static RenderStyle CreateStyle();

  std::vector<InlineBox*> boxes_;
  text::Offset offset_;
  text::Offset start_;
  const RenderStyle& style_;

  DISALLOW_COPY_AND_ASSIGN(LineBuilder);
};

LineBuilder::LineBuilder(text::Offset start, const RenderStyle& style)
    : offset_(start), style_(style), start_(start) {}

void LineBuilder::AddCodeUnit(const base::char16 code_unit) {
  base::string16 text = L"uFFFF";
  boxes_.push_back(new InlineUnicodeBox(
      style_, WidthOf(text), style_.font().height() + 4, offset_, text));
  offset_ += text::OffsetDelta(1);
}

void LineBuilder::AddMarker(TextMarker marker) {
  boxes_.push_back(new InlineMarkerBox(
      style_, WidthOf(L"x"), style_.font().height(), offset_, marker));
  offset_ += text::OffsetDelta(1);
}

void LineBuilder::AddText(const base::string16& text) {
  boxes_.push_back(new InlineTextBox(style_, WidthOf(text),
                                     style_.font().height(), offset_, text));
  offset_ += text::OffsetDelta(text.size());
}

RenderStyle CreateStyle() {
  css::Style css_style;
  css_style.set_bgcolor(css::Color());
  css_style.set_color(css::Color());
  css_style.set_font_family(L"Consolas");
  css_style.set_font_weight(css::FontWeight::Normal);
  css_style.set_font_size(10);
  css_style.set_font_style(css::FontStyle::Normal);
  css_style.set_text_decoration(css::TextDecoration::None);
  auto const font = FontSet::GetFont(css_style, 'x');
  return RenderStyle(css_style, font);
}

float LineBuilder::WidthOf(const base::string16& text) const {
  return ::ceil(style_.font().GetTextWidth(text));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// RootInlineBoxTest
//
class RootInlineBoxTest : public ::testing::Test {
 protected:
  RootInlineBoxTest();
  ~RootInlineBoxTest() override = default;

  const RenderStyle& style() const { return style_; }

  float WidthOf(const base::string16& text) const;

 private:
  RenderStyle style_;

  DISALLOW_COPY_AND_ASSIGN(RootInlineBoxTest);
};

RootInlineBoxTest::RootInlineBoxTest() : style_(CreateStyle()) {}

float RootInlineBoxTest::WidthOf(const base::string16& text) const {
  return ::ceil(style_.font().GetTextWidth(text));
}

TEST_F(RootInlineBoxTest, HitTestTextPosition) {
  LineBuilder line(text::Offset(100), style());
  line.AddText(L"foo");
  line.AddText(L"bar");
  line.AddText(L"baz");
  line.AddMarker(TextMarker::EndOfLine);
  const auto root_box = make_scoped_refptr(
      new RootInlineBox(line.boxes(), line.text_start(), line.text_end(),
                        line.ascent(), line.descent()));
  root_box->set_origin(gfx::PointF(10.0f, 20.0f));
  const auto height = root_box->height();
  const auto& size = gfx::SizeF(1.0f, height);
  EXPECT_EQ(gfx::RectF(), root_box->HitTestTextPosition(text::Offset(90)))
      << "If RootInlineBox contains specified offset, HitTestTextPosition "
         "returns empty rectangle";
  EXPECT_EQ(gfx::RectF(gfx::PointF(10.0f, 20.0f), size),
            root_box->HitTestTextPosition(text::Offset(100)));
  EXPECT_EQ(gfx::RectF(gfx::PointF(10.0f + WidthOf(L"f"), 20.0f), size),
            root_box->HitTestTextPosition(text::Offset(101)));
  EXPECT_EQ(gfx::RectF(gfx::PointF(10.0f + WidthOf(L"fo"), 20.0f), size),
            root_box->HitTestTextPosition(text::Offset(102)));
  EXPECT_EQ(gfx::RectF(gfx::PointF(10.0f + WidthOf(L"foo"), 20.0f), size),
            root_box->HitTestTextPosition(text::Offset(103)));
  EXPECT_EQ(
      gfx::RectF(gfx::PointF(10.0f + WidthOf(L"foo") + WidthOf(L"b"), 20.0f),
                 size),
      root_box->HitTestTextPosition(text::Offset(104)));
  EXPECT_EQ(
      gfx::RectF(gfx::PointF(10.0f + WidthOf(L"foo") + WidthOf(L"ba"), 20.0f),
                 size),
      root_box->HitTestTextPosition(text::Offset(105)));
  EXPECT_EQ(gfx::RectF(gfx::PointF(10.0f + WidthOf(L"foo") + WidthOf(L"bar") +
                                       WidthOf(L"b"),
                                   20.0f),
                       size),
            root_box->HitTestTextPosition(text::Offset(107)));
  EXPECT_EQ(gfx::RectF(gfx::PointF(10.0f + WidthOf(L"foo") + WidthOf(L"bar") +
                                       WidthOf(L"baz"),
                                   20.0f),
                       size),
            root_box->HitTestTextPosition(text::Offset(109)));
  EXPECT_EQ(gfx::RectF(), root_box->HitTestTextPosition(text::Offset(110)))
      << "root_box contains 100 to 109.";
}

TEST_F(RootInlineBoxTest, HitTestTextPositionCodeUnit) {
  LineBuilder line(text::Offset(100), style());
  line.AddText(L"a");
  line.AddCodeUnit(0xFFFF);
  line.AddText(L"b");
  line.AddMarker(TextMarker::EndOfLine);
  const auto root_box = make_scoped_refptr(
      new RootInlineBox(line.boxes(), line.text_start(), line.text_end(),
                        line.ascent(), line.descent()));
  root_box->set_origin(gfx::PointF(10.0f, 20.0f));
  const auto height = root_box->height();
  const auto& size = gfx::SizeF(1.0f, height);
  EXPECT_EQ(
      gfx::RectF(gfx::PointF(10.0f + WidthOf(L"a"), 20.0f),
                 size),
      root_box->HitTestTextPosition(text::Offset(101)));
  EXPECT_EQ(
      gfx::RectF(gfx::PointF(10.0f + WidthOf(L"a") + WidthOf(L"uFFFF"), 20.0f),
                 size),
      root_box->HitTestTextPosition(text::Offset(102)));
}

TEST_F(RootInlineBoxTest, MapXToPosn) {
  LineBuilder line(text::Offset(100), style());
  line.AddText(L"foo");
  line.AddText(L"bar");
  line.AddText(L"baz");
  line.AddMarker(TextMarker::EndOfLine);
  const auto root_box = make_scoped_refptr(
      new RootInlineBox(line.boxes(), line.text_start(), line.text_end(),
                        line.ascent(), line.descent()));
  const auto width = root_box->width();
  EXPECT_EQ(100, root_box->MapXToPosn(0));
  EXPECT_EQ(100, root_box->MapXToPosn(WidthOf(L"f") / 2));
  EXPECT_EQ(100, root_box->MapXToPosn(WidthOf(L"f") - 1));
  EXPECT_EQ(101, root_box->MapXToPosn(WidthOf(L"fo") - 1));
  EXPECT_EQ(102, root_box->MapXToPosn(WidthOf(L"fo")));
  EXPECT_EQ(103, root_box->MapXToPosn(WidthOf(L"foo")));
  EXPECT_EQ(103, root_box->MapXToPosn(WidthOf(L"foo") + 1));
  EXPECT_EQ(105, root_box->MapXToPosn(WidthOf(L"fooba") + 1));
  EXPECT_EQ(108, root_box->MapXToPosn(WidthOf(L"foobarbaz") - 1));
  EXPECT_EQ(109, root_box->MapXToPosn(width));
}

TEST_F(RootInlineBoxTest, MapXToPosnCodeUnit) {
  LineBuilder line(text::Offset(100), style());
  line.AddText(L"a");
  line.AddCodeUnit(0xFFFF);
  line.AddText(L"b");
  line.AddMarker(TextMarker::EndOfLine);
  const auto root_box = make_scoped_refptr(
      new RootInlineBox(line.boxes(), line.text_start(), line.text_end(),
                        line.ascent(), line.descent()));
  const auto width = root_box->width();
  EXPECT_EQ(100, root_box->MapXToPosn(0));
  EXPECT_EQ(100, root_box->MapXToPosn(WidthOf(L"a") / 2));
  EXPECT_EQ(100, root_box->MapXToPosn(WidthOf(L"a") - 1));
  EXPECT_EQ(101, root_box->MapXToPosn(WidthOf(L"a")));
  EXPECT_EQ(101, root_box->MapXToPosn(WidthOf(L"au")));
  EXPECT_EQ(102, root_box->MapXToPosn(width - 10));
  EXPECT_EQ(103, root_box->MapXToPosn(width));
}

}  // namespace layout
