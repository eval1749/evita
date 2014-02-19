// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include <memory>

#pragma warning(push)
#pragma warning(disable: 4365 4625 4626 4826)
#include "gtest/gtest.h"
#pragma warning(pop)

#include "evita/text/buffer.h"
#include "evita/text/modes/plain_text_mode.h"
#include "evita/text/range.h"

namespace {

class BufferTest : public ::testing::Test {
  private: std::unique_ptr<text::Buffer> buffer_;

  protected: BufferTest()
      : buffer_(new text::Buffer(L"*test*", new text::PlainTextMode())) {
  }
  public: virtual ~BufferTest() {
  }

  public: text::Buffer* buffer() const { return buffer_.get(); }

  DISALLOW_COPY_AND_ASSIGN(BufferTest);
};

TEST_F(BufferTest, InsertAt) {
  buffer()->InsertBefore(0, L"abc");

  StyleValues style_values1 = {0};
  style_values1.m_rgfMask = StyleValues::Mask_Syntax;
  style_values1.SetSyntax(1);
  buffer()->SetStyle(1, 2, &style_values1);

  StyleValues style_values2 = {0};
  style_values2.m_rgfMask = StyleValues::Mask_Syntax;
  style_values2.SetSyntax(2);
  buffer()->SetStyle(2, 3, &style_values2);

  auto range = std::make_unique<text::Range>(buffer(), 2, 2);

  // Insert 'X' at 'c'.
  buffer()->Insert(2, L"X");
  EXPECT_EQ(4, buffer()->GetEnd());
  EXPECT_EQ('X', buffer()->GetCharAt(2));
  EXPECT_EQ('c', buffer()->GetCharAt(3));
  EXPECT_EQ(2, buffer()->GetStyleAt(2)->GetSyntax()) <<
      "The style of inserted text is the style of insertion position.";
  EXPECT_EQ(2, buffer()->GetStyleAt(3)->GetSyntax()) <<
      "The style at insertion position isn't changed.";
  EXPECT_EQ(2, range->GetStart()) <<
    "The range at insertion position should not be moved.";
  EXPECT_EQ(2, range->GetEnd()) <<
    "The range at insertion position should not be moved.";
}

TEST_F(BufferTest, InsertBefore) {
  buffer()->InsertBefore(0, L"abc");

  StyleValues style_values1 = {0};
  style_values1.m_rgfMask = StyleValues::Mask_Syntax;
  style_values1.SetSyntax(1);
  buffer()->SetStyle(1, 2, &style_values1);

  StyleValues style_values2 = {0};
  style_values2.m_rgfMask = StyleValues::Mask_Syntax;
  style_values2.SetSyntax(2);
  buffer()->SetStyle(2, 3, &style_values2);

  auto range = std::make_unique<text::Range>(buffer(), 2, 2);

  // Insert 'X' before 'c'.
  buffer()->InsertBefore(2, L"X");
  EXPECT_EQ(4, buffer()->GetEnd());
  EXPECT_EQ('X', buffer()->GetCharAt(2));
  EXPECT_EQ('c', buffer()->GetCharAt(3));
  EXPECT_EQ(1, buffer()->GetStyleAt(2)->GetSyntax()) <<
      "The style of inserted text is inherited from styles before insertion"
      " position.";
  EXPECT_EQ(2, buffer()->GetStyleAt(3)->GetSyntax()) <<
      "The style at insertion position isn't changed.";
  EXPECT_EQ(3, range->GetStart()) <<
    "The range at insertion position should be push back.";
  EXPECT_EQ(3, range->GetEnd()) <<
    "The range at insertion position should be push back.";
}

TEST_F(BufferTest, OperatorEqual) {
  std::unique_ptr<text::Buffer> other_buffer;
  auto& buffer_ref = *buffer();
  EXPECT_TRUE(buffer_ref == buffer());
  EXPECT_TRUE(buffer_ref == buffer_ref);
  EXPECT_FALSE(buffer_ref == *other_buffer);
}

TEST_F(BufferTest, OperatorNotEqual) {
  std::unique_ptr<text::Buffer> other_buffer;
  auto& buffer_ref = *buffer();
  EXPECT_FALSE(buffer_ref != buffer());
  EXPECT_FALSE(buffer_ref != buffer_ref);
  EXPECT_TRUE(buffer_ref != *other_buffer);
}

}  // namespace
