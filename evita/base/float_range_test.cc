// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/base/float_range.h"

#include "gtest/gtest.h"

namespace base {

TEST(FloatRangeTest, empty) {
  EXPECT_TRUE(FloatRange().empty());
  EXPECT_TRUE(FloatRange(1, 1).empty());
  EXPECT_FALSE(FloatRange(1, 2).empty());
}

TEST(FloatRangeTest, getters) {
  FloatRange range1(1, 4);

  EXPECT_EQ(1, range1.lower());
  EXPECT_EQ(4, range1.upper());
  EXPECT_EQ(3, range1.length());
}

TEST(FloatRangeTest, Equals) {
  FloatRange range0;
  FloatRange range1(1, 2);
  FloatRange range2(2, 3);
  FloatRange range3(1, 2);

  EXPECT_TRUE(range0 == range0);
  EXPECT_FALSE(range0 == range1);
  EXPECT_FALSE(range0 == range2);
  EXPECT_FALSE(range0 == range3);

  EXPECT_FALSE(range1 == range0);
  EXPECT_TRUE(range1 == range1);
  EXPECT_FALSE(range1 == range2);
  EXPECT_TRUE(range1 == range3);

  EXPECT_FALSE(range2 == range0);
  EXPECT_FALSE(range2 == range1);
  EXPECT_TRUE(range2 == range2);
  EXPECT_FALSE(range2 == range3);

  EXPECT_FALSE(range3 == range0);
  EXPECT_TRUE(range3 == range1);
  EXPECT_FALSE(range3 == range2);
  EXPECT_TRUE(range3 == range3);
}

}  // namespace base
