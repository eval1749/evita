// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/geometry/float_rect.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(FloatRectTest, Equals) {
  FloatRect rect1(FloatPoint(1, 2), FloatSize(4, 5));
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

}  // namespace visuals
