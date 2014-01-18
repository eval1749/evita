// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include <memory>

#pragma warning(push)
#pragma warning(disable: 4365 4625 4626 4826)
#include "gtest/gtest.h"
#pragma warning(pop)

#include "evita/text/buffer.h"

namespace {

class BufferTest : public ::testing::Test {
  private: std::unique_ptr<text::Buffer> buffer_;

  protected: BufferTest() : buffer_(new text::Buffer(L"*test*")) {
  }
  public: virtual ~BufferTest() {
  }

  public: text::Buffer& buffer() const { return *buffer_; }

  DISALLOW_COPY_AND_ASSIGN(BufferTest);
};

TEST_F(BufferTest, OperatorEqual) {
  std::unique_ptr<text::Buffer> other_buffer;
  EXPECT_TRUE(buffer() == &buffer());
  EXPECT_TRUE(buffer() == buffer());
  EXPECT_FALSE(buffer() == *other_buffer);
}

TEST_F(BufferTest, OperatorNotEqual) {
  std::unique_ptr<text::Buffer> other_buffer;
  EXPECT_FALSE(buffer() != &buffer());
  EXPECT_FALSE(buffer() != buffer());
  EXPECT_TRUE(buffer() != *other_buffer);
}

}  // namespace
