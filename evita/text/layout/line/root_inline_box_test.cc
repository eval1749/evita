// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <cmath>
#include <vector>

#include "evita/gfx/font.h"
#include "evita/gfx/font_face.h"
#include "evita/text/layout/line/inline_box.h"
#include "evita/text/layout/line/root_inline_box.h"
#include "evita/text/style/computed_style_builder.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace layout {

namespace {

const auto kLeadingWidth = 10.0f;

//////////////////////////////////////////////////////////////////////
//
// LineBuilder
//
class LineBuilder final {
 public:
  LineBuilder(text::Offset start, const ComputedStyle& style);
  ~LineBuilder() = default;

  void AddCodeUnit(const base::char16 code_unit);
  void AddMarker(TextMarker marker);
  void AddText(const base::string16& text);
  std::unique_ptr<RootInlineBox> Build() const;
  float WidthOf(const base::string16& text) const;

 private:
  void AddBoxInternal(InlineBox* box);
  static ComputedStyle CreateStyle();

  float ascent_ = 0;
  std::vector<InlineBox*> boxes_;
  float descent_ = 0;
  const gfx::Font& font_;
  float left_ = 0;
  text::OffsetDelta offset_;
  text::Offset start_;
  const ComputedStyle& style_;

  DISALLOW_COPY_AND_ASSIGN(LineBuilder);
};

LineBuilder::LineBuilder(text::Offset start, const ComputedStyle& style)
    : font_(*style.fonts()[0]), style_(style), start_(start) {
  AddBoxInternal(
      new InlineFillerBox(style_, 0, kLeadingWidth, 10, text::OffsetDelta(0)));
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
  AddBoxInternal(new InlineUnicodeBox(style_, font_, left_, WidthOf(text),
                                      font_.height() + 4, offset_, text));
}

void LineBuilder::AddMarker(TextMarker marker) {
  const auto next_offset =
      marker == TextMarker::LineWrap ? offset_ : offset_ + text::OffsetDelta(1);
  AddBoxInternal(new InlineMarkerBox(style_, font_, left_, WidthOf(L"x"),
                                     font_.height(), offset_, next_offset,
                                     marker));
}

void LineBuilder::AddText(const base::string16& text) {
  AddBoxInternal(new InlineTextBox(style_, font_, left_, WidthOf(text),
                                   font_.height(), offset_, text));
}

std::unique_ptr<RootInlineBox> LineBuilder::Build() const {
  return std::make_unique<RootInlineBox>(boxes_, start_, start_,
                                         start_ + offset_, ascent_, descent_);
}

gfx::RectF CaretBoundsOf(int origin_x, int origin_y, int height) {
  return gfx::RectF(
      gfx::PointF(static_cast<float>(origin_x), static_cast<float>(origin_y)),
      gfx::SizeF(1.0f, static_cast<float>(height)));
}

std::unique_ptr<ComputedStyle> CreateStyle() {
  gfx::FontProperties props;
  props.family_name = L"Consolas";
  props.font_size_pt = 10;
  const auto& font = gfx::Font::Get(props);
  return ComputedStyle::Builder()
      .SetBackgroundColor(gfx::ColorF(1, 1, 1))
      .SetColor(gfx::ColorF(0, 0, 0))
      .SetFonts({&font})
      .Build();
}

float LineBuilder::WidthOf(const base::string16& text) const {
  return ::ceil(font_.GetTextWidth(text));
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

  const ComputedStyle& style() const { return *style_; }

  gfx::PointF PointFor(const RootInlineBox& line,
                       const std::vector<base::string16>& texts) const;
  float WidthOf(const base::string16& text) const;

 private:
  const std::unique_ptr<ComputedStyle> style_;

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
  return ::ceil(style().fonts()[0]->GetTextWidth(text));
}

TEST_F(RootInlineBoxTest, HitTestPoint) {
  LineBuilder builder(text::Offset(100), style());
  builder.AddText(L"foo");
  builder.AddText(L"bar");
  builder.AddText(L"baz");
  builder.AddMarker(TextMarker::EndOfLine);
  const auto& root_box = builder.Build();
  const auto width = root_box->width();
  EXPECT_EQ(text::Offset(100), root_box->HitTestPoint(-1));
  EXPECT_EQ(text::Offset(100), root_box->HitTestPoint(0));
  EXPECT_EQ(text::Offset(100), root_box->HitTestPoint(kLeadingWidth));
  EXPECT_EQ(text::Offset(100),
            root_box->HitTestPoint(kLeadingWidth + WidthOf(L"f") / 2));
  EXPECT_EQ(text::Offset(100),
            root_box->HitTestPoint(kLeadingWidth + WidthOf(L"f") - 1));
  EXPECT_EQ(text::Offset(101),
            root_box->HitTestPoint(kLeadingWidth + WidthOf(L"fo") - 1));
  EXPECT_EQ(text::Offset(102),
            root_box->HitTestPoint(kLeadingWidth + WidthOf(L"fo")));
  EXPECT_EQ(text::Offset(103),
            root_box->HitTestPoint(kLeadingWidth + WidthOf(L"foo")));
  EXPECT_EQ(text::Offset(103),
            root_box->HitTestPoint(kLeadingWidth + WidthOf(L"foo") + 1));
  EXPECT_EQ(text::Offset(105),
            root_box->HitTestPoint(kLeadingWidth + WidthOf(L"fooba") + 1));
  EXPECT_EQ(text::Offset(108),
            root_box->HitTestPoint(kLeadingWidth + WidthOf(L"foobarbaz") - 1));
  EXPECT_EQ(text::Offset(109), root_box->HitTestPoint(kLeadingWidth + width));
  EXPECT_EQ(text::Offset(109), root_box->HitTestPoint(99999));
}

TEST_F(RootInlineBoxTest, HitTestPointCodeUnit) {
  LineBuilder builder(text::Offset(100), style());
  builder.AddText(L"a");
  builder.AddCodeUnit(0xFFFF);
  builder.AddText(L"b");
  builder.AddMarker(TextMarker::EndOfLine);
  const auto& root_box = builder.Build();
  const auto width = root_box->width();
  EXPECT_EQ(text::Offset(100), root_box->HitTestPoint(-1));
  EXPECT_EQ(text::Offset(100), root_box->HitTestPoint(0));
  EXPECT_EQ(text::Offset(100), root_box->HitTestPoint(kLeadingWidth));
  EXPECT_EQ(text::Offset(100),
            root_box->HitTestPoint(kLeadingWidth + WidthOf(L"a") / 2));
  EXPECT_EQ(text::Offset(100),
            root_box->HitTestPoint(kLeadingWidth + WidthOf(L"a") - 1));
  EXPECT_EQ(text::Offset(101),
            root_box->HitTestPoint(kLeadingWidth + WidthOf(L"a")));
  EXPECT_EQ(text::Offset(101),
            root_box->HitTestPoint(kLeadingWidth + WidthOf(L"au")));
  EXPECT_EQ(text::Offset(101),
            root_box->HitTestPoint(kLeadingWidth + WidthOf(L"auF")));
  EXPECT_EQ(text::Offset(101),
            root_box->HitTestPoint(kLeadingWidth + WidthOf(L"auFF")));
  EXPECT_EQ(text::Offset(101),
            root_box->HitTestPoint(kLeadingWidth + WidthOf(L"auFFF")));
  EXPECT_EQ(text::Offset(102), root_box->HitTestPoint(width - 10));
  EXPECT_EQ(text::Offset(103), root_box->HitTestPoint(width));
  EXPECT_EQ(text::Offset(103), root_box->HitTestPoint(99999));
}

TEST_F(RootInlineBoxTest, HitTestTextPosition) {
  LineBuilder builder(text::Offset(100), style());
  builder.AddText(L"foo");
  builder.AddText(L"bar");
  builder.AddText(L"baz");
  builder.AddMarker(TextMarker::EndOfLine);
  const auto& root_box = builder.Build();
  const auto& origin = gfx::PointF(100.0f, 200.0f);
  root_box->set_origin(origin);
  EXPECT_EQ(gfx::RectF(), root_box->HitTestTextPosition(text::Offset(90)))
      << "If RootInlineBox contains specified offset, HitTestTextPosition "
         "returns empty rectangle";
  EXPECT_EQ(CaretBoundsOf(110, 200, 16),
            root_box->HitTestTextPosition(text::Offset(100)));
  EXPECT_EQ(CaretBoundsOf(117, 200, 16),
            root_box->HitTestTextPosition(text::Offset(101)));
  EXPECT_EQ(CaretBoundsOf(124, 200, 16),
            root_box->HitTestTextPosition(text::Offset(102)));
  EXPECT_EQ(CaretBoundsOf(131, 200, 16),
            root_box->HitTestTextPosition(text::Offset(103)));
  EXPECT_EQ(CaretBoundsOf(138, 200, 16),
            root_box->HitTestTextPosition(text::Offset(104)));
  EXPECT_EQ(CaretBoundsOf(145, 200, 16),
            root_box->HitTestTextPosition(text::Offset(105)));
  EXPECT_EQ(CaretBoundsOf(159, 200, 16),
            root_box->HitTestTextPosition(text::Offset(107)));
  EXPECT_EQ(CaretBoundsOf(173, 200, 16),
            root_box->HitTestTextPosition(text::Offset(109)));
  EXPECT_EQ(gfx::RectF(), root_box->HitTestTextPosition(text::Offset(110)))
      << "root_box contains 100 to 109.";
}

TEST_F(RootInlineBoxTest, HitTestTextPositionCodeUnit) {
  LineBuilder builder(text::Offset(100), style());
  builder.AddText(L"a");
  builder.AddCodeUnit(0xFFFF);
  builder.AddText(L"b");
  builder.AddMarker(TextMarker::EndOfLine);
  const auto& root_box = builder.Build();
  root_box->set_origin(gfx::PointF(10.0f, 20.0f));
  EXPECT_EQ(CaretBoundsOf(20, 24, 16),
            root_box->HitTestTextPosition(text::Offset(100)));
  EXPECT_EQ(CaretBoundsOf(27, 20, 20),
            root_box->HitTestTextPosition(text::Offset(101)));
  EXPECT_EQ(CaretBoundsOf(62, 24, 16),
            root_box->HitTestTextPosition(text::Offset(102)));
}

}  // namespace layout
