// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <cmath>
#include <vector>

#include "evita/css/style.h"
#include "evita/gfx/font.h"
#include "evita/layout/line/inline_box.h"
#include "evita/layout/line/root_inline_box.h"
#include "evita/layout/render_font_set.h"
#include "gtest/gtest.h"

namespace layout {

namespace {

const auto kLeadingWidth = 10.0f;

//////////////////////////////////////////////////////////////////////
//
// LineBuilder
//
class LineBuilder final {
 public:
  LineBuilder(text::Offset start, const RenderStyle& style);
  ~LineBuilder() = default;

  float ascent() const { return ::ceil(ascent_); }
  const std::vector<InlineBox*> boxes() const { return boxes_; }
  float descent() const { return ::ceil(descent_); }
  text::Offset text_end() const { return offset_; }
  text::Offset text_start() const { return start_; }

  void AddCodeUnit(const base::char16 code_unit);
  void AddMarker(TextMarker marker);
  void AddText(const base::string16& text);
  float WidthOf(const base::string16& text) const;

 private:
  void AddBoxInternal(InlineBox* box);
  static RenderStyle CreateStyle();

  float ascent_ = 0;
  std::vector<InlineBox*> boxes_;
  float descent_ = 0;
  float left_ = 0;
  text::Offset offset_;
  text::Offset start_;
  const RenderStyle& style_;

  DISALLOW_COPY_AND_ASSIGN(LineBuilder);
};

LineBuilder::LineBuilder(text::Offset start, const RenderStyle& style)
    : offset_(start), style_(style), start_(start) {
  AddBoxInternal(new InlineFillerBox(style_, 0, kLeadingWidth, 10, start));
}

void LineBuilder::AddBoxInternal(InlineBox* box) {
  boxes_.push_back(box);
  left_ += box->width();
  ascent_ = std::max(ascent_, box->ascent());
  descent_ = std::max(descent_, box->descent());
  offset_ = box->end();
}

void LineBuilder::AddCodeUnit(const base::char16 code_unit) {
  const auto next_offset = offset_ + text::OffsetDelta(1);
  base::string16 text = L"uFFFF";
  AddBoxInternal(new InlineUnicodeBox(
      style_, left_, WidthOf(text), style_.font().height() + 4, offset_, text));
}

void LineBuilder::AddMarker(TextMarker marker) {
  const auto next_offset =
      marker == TextMarker::LineWrap ? offset_ : offset_ + text::OffsetDelta(1);
  AddBoxInternal(new InlineMarkerBox(style_, left_, WidthOf(L"x"),
                                     style_.font().height(), offset_,
                                     next_offset, marker));
}

void LineBuilder::AddText(const base::string16& text) {
  AddBoxInternal(new InlineTextBox(style_, left_, WidthOf(text),
                                   style_.font().height(), offset_, text));
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
  return RenderStyle(css_style, *font);
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

  gfx::PointF PointFor(const RootInlineBox& line,
                       const std::vector<base::string16>& texts) const;
  float WidthOf(const base::string16& text) const;

 private:
  RenderStyle style_;

  DISALLOW_COPY_AND_ASSIGN(RootInlineBoxTest);
};

RootInlineBoxTest::RootInlineBoxTest() : style_(CreateStyle()) {}

gfx::PointF RootInlineBoxTest::PointFor(
    const RootInlineBox& root_box,
    const std::vector<base::string16>& texts) const {
  auto width = root_box.boxes().front()->width();
  for (const auto& text : texts)
    width += WidthOf(text);
  return root_box.origin() + gfx::SizeF(width, 0.0f);
}

float RootInlineBoxTest::WidthOf(const base::string16& text) const {
  return ::ceil(style_.font().GetTextWidth(text));
}

TEST_F(RootInlineBoxTest, HitTestPoint) {
  LineBuilder line(text::Offset(100), style());
  line.AddText(L"foo");
  line.AddText(L"bar");
  line.AddText(L"baz");
  line.AddMarker(TextMarker::EndOfLine);
  const auto root_box = make_scoped_refptr(
      new RootInlineBox(line.boxes(), line.text_start(), line.text_end(),
                        line.ascent(), line.descent()));
  const auto width = root_box->width();
  EXPECT_EQ(100, root_box->HitTestPoint(-1));
  EXPECT_EQ(100, root_box->HitTestPoint(0));
  EXPECT_EQ(100, root_box->HitTestPoint(kLeadingWidth));
  EXPECT_EQ(100, root_box->HitTestPoint(kLeadingWidth + WidthOf(L"f") / 2));
  EXPECT_EQ(100, root_box->HitTestPoint(kLeadingWidth + WidthOf(L"f") - 1));
  EXPECT_EQ(101, root_box->HitTestPoint(kLeadingWidth + WidthOf(L"fo") - 1));
  EXPECT_EQ(102, root_box->HitTestPoint(kLeadingWidth + WidthOf(L"fo")));
  EXPECT_EQ(103, root_box->HitTestPoint(kLeadingWidth + WidthOf(L"foo")));
  EXPECT_EQ(103, root_box->HitTestPoint(kLeadingWidth + WidthOf(L"foo") + 1));
  EXPECT_EQ(105, root_box->HitTestPoint(kLeadingWidth + WidthOf(L"fooba") + 1));
  EXPECT_EQ(108,
            root_box->HitTestPoint(kLeadingWidth + WidthOf(L"foobarbaz") - 1));
  EXPECT_EQ(109, root_box->HitTestPoint(kLeadingWidth + width));
  EXPECT_EQ(109, root_box->HitTestPoint(99999));
}

TEST_F(RootInlineBoxTest, HitTestPointCodeUnit) {
  LineBuilder line(text::Offset(100), style());
  line.AddText(L"a");
  line.AddCodeUnit(0xFFFF);
  line.AddText(L"b");
  line.AddMarker(TextMarker::EndOfLine);
  const auto root_box = make_scoped_refptr(
      new RootInlineBox(line.boxes(), line.text_start(), line.text_end(),
                        line.ascent(), line.descent()));
  const auto width = root_box->width();
  EXPECT_EQ(100, root_box->HitTestPoint(-1));
  EXPECT_EQ(100, root_box->HitTestPoint(0));
  EXPECT_EQ(100, root_box->HitTestPoint(kLeadingWidth));
  EXPECT_EQ(100, root_box->HitTestPoint(kLeadingWidth + WidthOf(L"a") / 2));
  EXPECT_EQ(100, root_box->HitTestPoint(kLeadingWidth + WidthOf(L"a") - 1));
  EXPECT_EQ(101, root_box->HitTestPoint(kLeadingWidth + WidthOf(L"a")));
  EXPECT_EQ(101, root_box->HitTestPoint(kLeadingWidth + WidthOf(L"au")));
  EXPECT_EQ(101, root_box->HitTestPoint(kLeadingWidth + WidthOf(L"auF")));
  EXPECT_EQ(101, root_box->HitTestPoint(kLeadingWidth + WidthOf(L"auFF")));
  EXPECT_EQ(101, root_box->HitTestPoint(kLeadingWidth + WidthOf(L"auFFF")));
  EXPECT_EQ(102, root_box->HitTestPoint(width - 10));
  EXPECT_EQ(103, root_box->HitTestPoint(width));
  EXPECT_EQ(103, root_box->HitTestPoint(99999));
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
  const auto& origin = gfx::PointF(100.0f, 200.0f);
  root_box->set_origin(origin);
  const auto height = root_box->height();
  const auto& size = gfx::SizeF(1.0f, height);
  EXPECT_EQ(gfx::RectF(), root_box->HitTestTextPosition(text::Offset(90)))
      << "If RootInlineBox contains specified offset, HitTestTextPosition "
         "returns empty rectangle";
  EXPECT_EQ(gfx::RectF(PointFor(*root_box, {}), size),
            root_box->HitTestTextPosition(text::Offset(100)));
  EXPECT_EQ(gfx::RectF(PointFor(*root_box, {L"f"}), size),
            root_box->HitTestTextPosition(text::Offset(101)));
  EXPECT_EQ(gfx::RectF(PointFor(*root_box, {L"fo"}), size),
            root_box->HitTestTextPosition(text::Offset(102)));
  EXPECT_EQ(gfx::RectF(PointFor(*root_box, {L"foo"}), size),
            root_box->HitTestTextPosition(text::Offset(103)));
  EXPECT_EQ(gfx::RectF(PointFor(*root_box, {L"foo", L"b"}), size),
            root_box->HitTestTextPosition(text::Offset(104)));
  EXPECT_EQ(gfx::RectF(PointFor(*root_box, {L"foo", L"ba"}), size),
            root_box->HitTestTextPosition(text::Offset(105)));
  EXPECT_EQ(gfx::RectF(PointFor(*root_box, {L"foo", L"bar", L"b"}), size),
            root_box->HitTestTextPosition(text::Offset(107)));
  EXPECT_EQ(gfx::RectF(PointFor(*root_box, {L"foo", L"bar", L"baz"}), size),
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
  EXPECT_EQ(gfx::RectF(PointFor(*root_box, {}) + gfx::SizeF(0.0f, 4.0f),
                       gfx::SizeF(1.0f, 16.0f)),
            root_box->HitTestTextPosition(text::Offset(100)));
  EXPECT_EQ(gfx::RectF(PointFor(*root_box, {L"a"}), size),
            root_box->HitTestTextPosition(text::Offset(101)));
  EXPECT_EQ(
      gfx::RectF(PointFor(*root_box, {L"a", L"uFFFF"}) + gfx::SizeF(0.0f, 4.0f),
                 gfx::SizeF(1.0f, 16.0f)),
      root_box->HitTestTextPosition(text::Offset(102)));
}

}  // namespace layout
