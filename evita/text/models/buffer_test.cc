// Copyright (c) 1996-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <sstream>
#include <string>

#include "base/strings/utf_string_conversions.h"
#include "evita/text/models/buffer.h"
#include "evita/text/models/marker.h"
#include "evita/text/models/marker_set.h"
#include "evita/text/models/offset.h"
#include "evita/text/models/range.h"
#include "evita/text/models/static_range.h"
#include "evita/text/style/models/style.h"
#include "testing/gtest/include/gtest/gtest.h"

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
  base::StringPiece16 GetMarkerAt(int offset) const;
  void SetMarker(int start, int end, base::StringPiece16 marker);
  void StartObserve();

 private:
  // BufferMutationObserver
  void DidChangeStyle(const StaticRange& range) final;

  std::unique_ptr<text::Buffer> buffer_;
  std::ostringstream style_changes_;

  DISALLOW_COPY_AND_ASSIGN(BufferTest);
};

void BufferTest::EndObserve() {
  buffer_->RemoveObserver(this);
}

base::StringPiece16 BufferTest::GetMarkerAt(int offset) const {
  const auto marker = buffer_->syntax_markers()->GetMarkerAt(Offset(offset));
  return marker ? marker->type().value() : L"";
}

void BufferTest::StartObserve() {
  style_changes_ = std::ostringstream();
  buffer_->AddObserver(this);
}

void BufferTest::SetMarker(int start, int end, base::StringPiece16 marker) {
  buffer_->syntax_markers()->InsertMarker(
      StaticRange(*buffer_, Offset(start), Offset(end)),
      base::AtomicString(marker));
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

  SetMarker(1, 2, L"marker1");
  SetMarker(2, 3, L"marker2");

  auto range = std::make_unique<text::Range>(buffer(), Offset(2), Offset(2));

  // Insert 'X' before 'c'.
  buffer()->InsertBefore(Offset(2), base::ASCIIToUTF16("X"));
  EXPECT_EQ(Offset(4), buffer()->GetEnd());
  EXPECT_EQ('X', buffer()->GetCharAt(Offset(2)));
  EXPECT_EQ('c', buffer()->GetCharAt(Offset(3)));
  EXPECT_EQ(L"marker1", GetMarkerAt(2))
      << "The style of inserted text is inherited from styles before insertion"
         " position.";
  EXPECT_EQ(L"marker2", GetMarkerAt(3))
      << "The style at insertion position isn't changed.";
  EXPECT_EQ(Offset(3), range->start())
      << "The range at insertion position should be push back.";
  EXPECT_EQ(Offset(3), range->end())
      << "The range at insertion position should be push back.";
}

TEST_F(BufferTest, Replace) {
  buffer()->Replace(Offset(0), Offset(0), base::ASCIIToUTF16("abc"));
  EXPECT_EQ(L"abc", buffer()->GetText(Offset(0), buffer()->GetEnd()));

  buffer()->Replace(Offset(1), Offset(2), base::ASCIIToUTF16("XY"));
  EXPECT_EQ(L"aXYc", buffer()->GetText(Offset(0), buffer()->GetEnd()));

  buffer()->Replace(Offset(1), Offset(3), base::ASCIIToUTF16(""));
  EXPECT_EQ(L"ac", buffer()->GetText(Offset(0), buffer()->GetEnd()));

  buffer()->Replace(Offset(2), Offset(2), base::ASCIIToUTF16("z"));
  EXPECT_EQ(L"acz", buffer()->GetText(Offset(0), buffer()->GetEnd()));
}

TEST_F(BufferTest, SetMarker) {
  buffer()->InsertBefore(Offset(0), base::ASCIIToUTF16("foo bar baz"));

  // Set all text font size to 30.
  SetMarker(0, 11, L"size30");

  // Color "bar" to red.
  SetMarker(4, 7, L"red");

  EXPECT_EQ(L"size30", GetMarkerAt(0));
  EXPECT_EQ(L"red", GetMarkerAt(4));

  const auto version = buffer()->version();
  SetMarker(4, 7, L"red");
  EXPECT_EQ(version, buffer()->version())
      << "Set same style doesn't change buffer";
}

TEST_F(BufferTest, SetMarkerWithObserver) {
  buffer()->InsertBefore(Offset(0), L"012345678");
  SetMarker(0, 9, L"marker1");

  StartObserve();
  SetMarker(2, 4, L"marker2");
  SetMarker(6, 8, L"marker2");
  EXPECT_EQ("2,4 6,8", style_changes());
  EndObserve();

  StartObserve();
  SetMarker(0, 9, L"marker2");
  EndObserve();
  EXPECT_EQ("0,2 4,6 8,9", style_changes());
}

}  // namespace text
