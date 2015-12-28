// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/geometry/float_rect.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(FloatRectTest, properties) {
  FloatRect rect1(FloatPoint(1, 2), FloatSize(3, 4));
  EXPECT_EQ(1, rect1.x());
  EXPECT_EQ(2, rect1.y());
  EXPECT_EQ(3, rect1.width());
  EXPECT_EQ(4, rect1.height());
  EXPECT_EQ(4, rect1.right());
  EXPECT_EQ(6, rect1.bottom());
  EXPECT_EQ(FloatPoint(1, 2), rect1.origin());
  EXPECT_EQ(FloatSize(3, 4), rect1.size());
  EXPECT_EQ(FloatPoint(4, 2), rect1.top_right());
  EXPECT_EQ(FloatPoint(1, 6), rect1.bottom_left());
  EXPECT_EQ(FloatPoint(4, 6), rect1.bottom_right());
}

TEST(FloatRectTest, Equals) {
  FloatRect rect1(FloatPoint(1, 2), FloatSize(3, 4));
  FloatRect rect2(FloatPoint(1, 2), FloatSize(5, 6));

  EXPECT_TRUE(FloatRect() == FloatRect());
  EXPECT_FALSE(FloatRect() != FloatRect());
  EXPECT_FALSE(FloatRect() == rect1);
  EXPECT_TRUE(FloatRect() != rect1);
  EXPECT_TRUE(rect1 == rect1);
  EXPECT_FALSE(rect1 != rect1);
  EXPECT_FALSE(rect1 == rect2);
  EXPECT_TRUE(rect1 != rect2);
}

TEST(FloatFloatRectTest, Intersect) {
  EXPECT_EQ(FloatRect(FloatPoint(10, 20), FloatSize(30, 40)),
            FloatRect(FloatPoint(10, 20), FloatSize(30, 40))
                .Intersect(FloatRect(FloatSize(100, 100))));
  EXPECT_EQ(FloatRect(),
            FloatRect(FloatPoint(10, 20), FloatSize(30, 40))
                .Intersect(FloatRect(FloatSize(1, 1))));
}

TEST(FloatFloatRectTest, Intersects) {
  EXPECT_TRUE(
      FloatRect(FloatPoint(10, 20), FloatSize(30, 40))
          .Intersects(FloatRect(FloatSize(100, 100))));
  EXPECT_FALSE(FloatRect(FloatPoint(10, 20), FloatSize(30, 40))
                   .Intersects(FloatRect(FloatSize(1, 1))));
}

}  // namespace visuals
