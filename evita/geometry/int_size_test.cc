// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/geometry/int_size.h"
#include "gtest/gtest.h"

namespace evita {

TEST(IntSizeTest, Equals) {
  EXPECT_TRUE(IntSize(0, 0) == IntSize(0, 0));
  EXPECT_TRUE(IntSize(1, 2) == IntSize(1, 2));
  EXPECT_FALSE(IntSize(0, 0) == IntSize(0, 1));
  EXPECT_FALSE(IntSize(0, 0) == IntSize(1, 0));
}

TEST(IntSizeTest, IsEmpty) {
  EXPECT_TRUE(IntSize().IsEmpty());
  EXPECT_TRUE(IntSize(0, 1).IsEmpty());
  EXPECT_TRUE(IntSize(1, 0).IsEmpty());
  EXPECT_FALSE(IntSize(1, 1).IsEmpty());
}

}  // namespace evita
