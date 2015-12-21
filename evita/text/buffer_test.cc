// Copyright (c) 1996-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <sstream>
#include <string>

#include "base/strings/utf_string_conversions.h"
#include "evita/css/style.h"
#include "evita/text/buffer.h"
#include "evita/text/offset.h"
#include "evita/text/range.h"
#include "evita/text/static_range.h"
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

class BufferTest : public ::testing::Test, public BufferMutationObserver {
 protected:
  BufferTest() : buffer_(new text::Buffer()) {}

  text::Buffer* buffer() const { return buffer_.get(); }
  const std::string style_changes() const { return style_changes_.str(); }

  MyLineAndColumn GetLineAndColumn(int offset) const {
    return MyLineAndColumn(buffer()->GetLineAndColumn(Offset(offset)));
  }

  void EndObserve();
  void StartObserve();

 private:
  // BufferMutationObserver
  void DidChangeStyle(const StaticRange& range) final;

  std::unique_ptr<text::Buffer> buffer_;
  std::ostringstream style_changes_;

  DISALLOW_COPY_AND_ASSIGN(BufferTest);
};

void BufferTest::StartObserve() {
  style_changes_ = std::ostringstream();
  buffer_->AddObserver(this);
}

void BufferTest::EndObserve() {
  buffer_->RemoveObserver(this);
}

// BufferMutationObserver
void BufferTest::DidChangeStyle(const StaticRange& range) {
  if (style_changes_.tellp())
    style_changes_ << " ";
  style_changes_ << range.start().value() << "," << range.end().value();
}

TEST_F(BufferTest, GetLineAndColumn) {
  buffer()->InsertBefore(Offset(0), base::ASCIIToUTF16("01\n02\n030405\n"));
  // 012_345_678901_
  // 01\n02\n030405\n
  EXPECT_EQ(MyLineAndColumn(1, 0), GetLineAndColumn(0));
  // No cache update.
  EXPECT_EQ(MyLineAndColumn(1, 1), GetLineAndColumn(1));
  // Populate cache.
  EXPECT_EQ(MyLineAndColumn(3, 0), GetLineAndColumn(6));
  EXPECT_EQ(MyLineAndColumn(1, 1), GetLineAndColumn(1));
  EXPECT_EQ(MyLineAndColumn(1, 2), GetLineAndColumn(2));
  EXPECT_EQ(MyLineAndColumn(2, 0), GetLineAndColumn(3));

  // Delete one character from line 2
  buffer()->Delete(Offset(3), Offset(4));
  // 012_34_5678901_
  // 01\n2\n030405\n
  EXPECT_EQ(MyLineAndColumn(3, 1), GetLineAndColumn(6));

  // Delete line 2
  buffer()->Delete(Offset(3), Offset(5));
  // 012_345678901_
  // 01\n030405\n
  EXPECT_EQ(MyLineAndColumn(2, 3), GetLineAndColumn(6));

  // Insert line 2
  buffer()->InsertBefore(Offset(3), L"02\n");
  // 012_345_678901_
  // 01\n02\n030405\n
  EXPECT_EQ(MyLineAndColumn(3, 0), GetLineAndColumn(6));
}

TEST_F(BufferTest, InsertBefore) {
  buffer()->InsertBefore(Offset(0), base::ASCIIToUTF16("abc"));

  css::Style style_values1;
  style_values1.set_text_decoration(css::TextDecoration::GreenWave);
  buffer()->SetStyle(Offset(1), Offset(2), style_values1);

  css::Style style_values2;
  style_values2.set_text_decoration(css::TextDecoration::RedWave);
  buffer()->SetStyle(Offset(2), Offset(3), style_values2);

  auto range = std::make_unique<text::Range>(buffer(), Offset(2), Offset(2));

  // Insert 'X' before 'c'.
  buffer()->InsertBefore(Offset(2), base::ASCIIToUTF16("X"));
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
  buffer()->InsertBefore(Offset(0), base::ASCIIToUTF16("foo bar baz"));

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

  const auto version = buffer()->version();
  buffer()->SetStyle(Offset(4), Offset(7), style_color_red);
  EXPECT_EQ(version, buffer()->version())
      << "Set same style doesn't change buffer";
}

TEST_F(BufferTest, SetStyleWithObserver) {
  buffer()->InsertBefore(Offset(0), L"012345678");
  css::Style style1(css::Color(1, 0, 0), css::Color(2, 0, 0));
  buffer()->SetStyle(text::Offset(0), text::Offset(9), style1);

  css::Style style2(css::Color(3, 0, 0), css::Color(2, 0, 0));
  StartObserve();
  buffer()->SetStyle(text::Offset(2), text::Offset(4), style2);
  buffer()->SetStyle(text::Offset(6), text::Offset(8), style2);
  EXPECT_EQ("2,4 6,8", style_changes());
  EndObserve();

  StartObserve();
  buffer()->SetStyle(text::Offset(0), text::Offset(9), style2);
  EndObserve();
  EXPECT_EQ("0,2 4,6 8,9", style_changes());
}

}  // namespace text
