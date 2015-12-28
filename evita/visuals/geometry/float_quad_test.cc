// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/geometry/float_quad.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(FloatQuadTest, Equals) {
  FloatQuad quad0;
  FloatQuad quad1(
      {FloatPoint(1, 1), FloatPoint(2, 2), FloatPoint(3, 3), FloatPoint(4, 4)});
  FloatQuad quad2(
      {FloatPoint(1, 1), FloatPoint(2, 2), FloatPoint(3, 3), FloatPoint(4, 4)});
  FloatQuad quad3(
      {FloatPoint(4, 4), FloatPoint(3, 3), FloatPoint(2, 2), FloatPoint(1, 1)});

  EXPECT_TRUE(quad0 == quad0);
  EXPECT_FALSE(quad0 == quad1);
  EXPECT_FALSE(quad0 == quad2);
  EXPECT_FALSE(quad0 == quad3);

  EXPECT_FALSE(quad0 != quad0);
  EXPECT_TRUE(quad0 != quad1);
  EXPECT_TRUE(quad0 != quad2);
  EXPECT_TRUE(quad0 != quad3);

  EXPECT_FALSE(quad1 == quad0);
  EXPECT_TRUE(quad1 == quad1);
  EXPECT_TRUE(quad1 == quad2);
  EXPECT_FALSE(quad1 == quad3);

  EXPECT_TRUE(quad1 != quad0);
  EXPECT_FALSE(quad1 != quad1);
  EXPECT_FALSE(quad1 != quad2);
  EXPECT_TRUE(quad1 != quad3);

  EXPECT_FALSE(quad2 == quad0);
  EXPECT_TRUE(quad2 == quad1);
  EXPECT_TRUE(quad2 == quad2);
  EXPECT_FALSE(quad2 == quad3);

  EXPECT_TRUE(quad2 != quad0);
  EXPECT_FALSE(quad2 != quad1);
  EXPECT_FALSE(quad2 != quad2);
  EXPECT_TRUE(quad2 != quad3);
}

}  // namespace visuals
