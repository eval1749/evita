// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#pragma warning(push)
#pragma warning(disable: 4365 4625 4626 4826)
#include "gtest/gtest.h"
#pragma warning(pop)

#include "evita/css/style.h"
#include "evita/text/buffer.h"
#include "evita/text/range.h"

namespace {
class RangeTest : public ::testing::Test {
  private: std::unique_ptr<text::Buffer> buffer_;

  protected: RangeTest()
      : buffer_(new text::Buffer()) {
  }
  public: virtual ~RangeTest() {
  }

  public: text::Buffer* buffer() const { return buffer_.get(); }

  DISALLOW_COPY_AND_ASSIGN(RangeTest);
};

TEST_F(RangeTest, SetText) {
  auto const range1 = std::make_unique<text::Range>(buffer(), 0, 0);
  range1->SetText(L"foo");
  EXPECT_EQ(0, range1->GetStart());
  EXPECT_EQ(3, range1->GetEnd());

  auto const range2 = std::make_unique<text::Range>(buffer(), 3, 3);
  auto const range3 = std::make_unique<text::Range>(buffer(), 0, 3);
  range2->SetText(L"bar");

  EXPECT_EQ(0, range1->GetStart());
  EXPECT_EQ(6, range1->GetEnd());

  EXPECT_EQ(3, range2->GetStart());
  EXPECT_EQ(6, range2->GetEnd());

  EXPECT_EQ(0, range3->GetStart());
  EXPECT_EQ(6, range3->GetEnd());
}

}  // namespace
