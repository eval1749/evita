// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#include "gtest/gtest.h"

#include "evita/core/buffer.h"
#include <memory>

namespace {

class BufferTest : public ::testing::Test {
  private: std::unique_ptr<Edit::Buffer> buffer_;

  protected: BufferTest() : buffer_(new Edit::Buffer(L"*test*")) {
  }
  public: virtual ~BufferTest() {
  }

  public: Edit::Buffer& buffer() const { return *buffer_; }
};

TEST_F(BufferTest, OperatorEqual) {
  std::unique_ptr<Edit::Buffer> other_buffer;
  EXPECT_TRUE(buffer() == &buffer());
  EXPECT_TRUE(buffer() == buffer());
  EXPECT_FALSE(buffer() == *other_buffer);
}

TEST_F(BufferTest, OperatorNotEqual) {
  std::unique_ptr<Edit::Buffer> other_buffer;
  EXPECT_FALSE(buffer() != &buffer());
  EXPECT_FALSE(buffer() != buffer());
  EXPECT_TRUE(buffer() != *other_buffer);
}

}  // namespace
