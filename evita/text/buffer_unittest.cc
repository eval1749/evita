// Copyright (c) 1996-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/css/style.h"
#include "evita/text/buffer.h"
#include "evita/text/offset.h"
#include "evita/text/range.h"
#include "gtest/gtest.h"

namespace {

struct MyLineAndColumn : text::LineAndColumn {
  int column;
  int line_number;

  MyLineAndColumn(int line_number_in, int column_in) {
    column = column_in;
    line_number = line_number_in;
  }

  explicit MyLineAndColumn(const text::LineAndColumn other) {
    column = other.column;
    line_number = other.line_number;
  }

  bool operator==(const MyLineAndColumn& other) const {
    return column == other.column && line_number == other.line_number;
  }

  bool operator!=(const MyLineAndColumn& other) const {
    return !operator==(other);
  }
};

}  // namespace

namespace std {
ostream& operator<<(ostream& ostream, const MyLineAndColumn& line_and_column) {
  return ostream << "(" << line_and_column.line_number << ", "
                 << line_and_column.column << ")";
}
}  // namespace std

namespace text {

class BufferTest : public ::testing::Test {
 public:
  text::Buffer* buffer() const { return buffer_.get(); }

 protected:
  BufferTest() : buffer_(new text::Buffer()) {}

  MyLineAndColumn GetLineAndColumn(int offset) const {
    return MyLineAndColumn(buffer()->GetLineAndColumn(Offset(offset)));
  }

 private:
  std::unique_ptr<text::Buffer> buffer_;

  DISALLOW_COPY_AND_ASSIGN(BufferTest);
};

TEST_F(BufferTest, GetLineAndColumn) {
  buffer()->Insert(Offset(0), L"01\n02\n03\04\05\n");
  EXPECT_EQ(MyLineAndColumn(1, 0), GetLineAndColumn(0));
  // No cache update.
  EXPECT_EQ(MyLineAndColumn(1, 1), GetLineAndColumn(1));
  // Populate cache.
  EXPECT_EQ(MyLineAndColumn(3, 0), GetLineAndColumn(6));
  EXPECT_EQ(MyLineAndColumn(1, 1), GetLineAndColumn(1));
  EXPECT_EQ(MyLineAndColumn(1, 2), GetLineAndColumn(2));
  EXPECT_EQ(MyLineAndColumn(2, 0), GetLineAndColumn(3));
}

TEST_F(BufferTest, InsertAt) {
  buffer()->InsertBefore(Offset(0), L"abc");

  css::Style style_values1;
  style_values1.set_text_decoration(css::TextDecoration::GreenWave);
  buffer()->SetStyle(Offset(1), Offset(2), style_values1);

  css::Style style_values2;
  style_values2.set_text_decoration(css::TextDecoration::RedWave);
  buffer()->SetStyle(Offset(2), Offset(3), style_values2);

  auto range = std::make_unique<text::Range>(buffer(), Offset(2), Offset(2));

  // Insert 'X' at 'c'.
  buffer()->Insert(Offset(2), L"X");
  EXPECT_EQ(Offset(4), buffer()->GetEnd());
  EXPECT_EQ('X', buffer()->GetCharAt(Offset(2)));
  EXPECT_EQ('c', buffer()->GetCharAt(Offset(3)));
  EXPECT_EQ(css::TextDecoration::RedWave,
            buffer()->GetStyleAt(Offset(2)).text_decoration())
      << "The style of inserted text is the style of insertion position.";
  EXPECT_EQ(css::TextDecoration::RedWave,
            buffer()->GetStyleAt(Offset(3)).text_decoration())
      << "The style at insertion position isn't changed.";
  EXPECT_EQ(Offset(2), range->start())
      << "The range at insertion position should not be moved.";
  EXPECT_EQ(Offset(2), range->end())
      << "The range at insertion position should not be moved.";
}

TEST_F(BufferTest, InsertBefore) {
  buffer()->InsertBefore(Offset(0), L"abc");

  css::Style style_values1;
  style_values1.set_text_decoration(css::TextDecoration::GreenWave);
  buffer()->SetStyle(Offset(1), Offset(2), style_values1);

  css::Style style_values2;
  style_values2.set_text_decoration(css::TextDecoration::RedWave);
  buffer()->SetStyle(Offset(2), Offset(3), style_values2);

  auto range = std::make_unique<text::Range>(buffer(), Offset(2), Offset(2));

  // Insert 'X' before 'c'.
  buffer()->InsertBefore(Offset(2), L"X");
  EXPECT_EQ(Offset(4), buffer()->GetEnd());
  EXPECT_EQ('X', buffer()->GetCharAt(Offset(2)));
  EXPECT_EQ('c', buffer()->GetCharAt(Offset(3)));
  EXPECT_EQ(css::TextDecoration::GreenWave,
            buffer()->GetStyleAt(Offset(2)).text_decoration())
      << "The style of inserted text is inherited from styles before insertion"
         " position.";
  EXPECT_EQ(css::TextDecoration::RedWave,
            buffer()->GetStyleAt(Offset(3)).text_decoration())
      << "The style at insertion position isn't changed.";
  EXPECT_EQ(Offset(3), range->start())
      << "The range at insertion position should be push back.";
  EXPECT_EQ(Offset(3), range->end())
      << "The range at insertion position should be push back.";
}

TEST_F(BufferTest, SetStyle) {
  buffer()->InsertBefore(Offset(0), L"foo bar baz");

  // Set all text font size to 30.
  css::Style style_font_size_30;
  style_font_size_30.set_font_size(30);
  buffer()->SetStyle(Offset(0), buffer()->GetEnd(), style_font_size_30);

  // Color "bar" to red.
  css::Style style_color_red;
  style_color_red.set_color(css::Color(0xCC, 0, 0));
  buffer()->SetStyle(Offset(4), Offset(7), style_color_red);

  EXPECT_EQ(30, buffer()->GetStyleAt(Offset(4)).font_size())
      << "Set color doesn't affect font size.";
}

}  // namespace text
