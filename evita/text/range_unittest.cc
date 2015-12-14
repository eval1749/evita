// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#pragma warning(push)
#pragma warning(disable : 4365 4625 4626 4826)
#include "gtest/gtest.h"
#pragma warning(pop)

#include "evita/css/style.h"
#include "evita/text/buffer.h"
#include "evita/text/range.h"

namespace text {

class RangeTest : public ::testing::Test {
 public:
  Buffer* buffer() const { return buffer_.get(); }

 protected:
  RangeTest() : buffer_(new Buffer()) {}

 private:
  std::unique_ptr<Buffer> buffer_;
  DISALLOW_COPY_AND_ASSIGN(RangeTest);
};

TEST_F(RangeTest, SetText) {
  auto const range1 = std::make_unique<Range>(buffer(), Offset(), Offset());
  range1->set_text(L"foo");
  EXPECT_EQ(Offset(0), range1->start());
  EXPECT_EQ(Offset(3), range1->end());

  auto const range2 = std::make_unique<Range>(buffer(), Offset(3), Offset(3));
  auto const range3 = std::make_unique<Range>(buffer(), Offset(0), Offset(3));
  range2->set_text(L"bar");

  EXPECT_EQ(Offset(0), range1->start());
  EXPECT_EQ(Offset(6), range1->end());

  EXPECT_EQ(Offset(3), range2->start());
  EXPECT_EQ(Offset(6), range2->end());

  EXPECT_EQ(Offset(0), range3->start());
  EXPECT_EQ(Offset(6), range3->end());
}

}  // namespace text
