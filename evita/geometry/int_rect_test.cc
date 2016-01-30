// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/geometry/int_rect.h"
#include "gtest/gtest.h"

namespace evita {

TEST(IntRectTest, properties) {
  IntRect rect1(IntPoint(1, 2), IntSize(3, 4));
  EXPECT_EQ(1, rect1.x());
  EXPECT_EQ(2, rect1.y());
  EXPECT_EQ(3, rect1.width());
  EXPECT_EQ(4, rect1.height());
  EXPECT_EQ(4, rect1.right());
  EXPECT_EQ(6, rect1.bottom());
  EXPECT_EQ(IntPoint(1, 2), rect1.origin());
  EXPECT_EQ(IntSize(3, 4), rect1.size());
  EXPECT_EQ(IntPoint(4, 2), rect1.top_right());
  EXPECT_EQ(IntPoint(1, 6), rect1.bottom_left());
  EXPECT_EQ(IntPoint(4, 6), rect1.bottom_right());
}

TEST(IntRectTest, Equals) {
  IntRect rect1(IntPoint(1, 2), IntSize(3, 4));
  IntRect rect2(IntPoint(1, 2), IntSize(5, 6));

  EXPECT_TRUE(IntRect() == IntRect());
  EXPECT_FALSE(IntRect() != IntRect());
  EXPECT_FALSE(IntRect() == rect1);
  EXPECT_TRUE(IntRect() != rect1);
  EXPECT_TRUE(rect1 == rect1);
  EXPECT_FALSE(rect1 != rect1);
  EXPECT_FALSE(rect1 == rect2);
  EXPECT_TRUE(rect1 != rect2);
}

}  // namespace evita
