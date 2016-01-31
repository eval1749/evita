// Copyright (c) 201g Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/base/colors/int8_color.h"
#include "gtest/gtest.h"

namespace base {

TEST(Int8ColorTest, Equals) {
  Int8Color color0;
  Int8Color color1(25, 50, 75, 100);
  Int8Color color2(128, 140, 200, 240);

  EXPECT_TRUE(color0 == color0);
  EXPECT_TRUE(color1 == color1);
  EXPECT_TRUE(color2 == color2);

  EXPECT_FALSE(color0 != color0);
  EXPECT_FALSE(color1 != color1);
  EXPECT_FALSE(color2 != color2);

  EXPECT_TRUE(color0 == Int8Color());
  EXPECT_TRUE(color1 == Int8Color(25, 50, 75, 100));
  EXPECT_TRUE(color2 == Int8Color(128, 140, 200, 240));

  EXPECT_FALSE(color0 != Int8Color());
  EXPECT_FALSE(color1 != Int8Color(25, 50, 75, 100));
  EXPECT_FALSE(color2 != Int8Color(128, 140, 200, 240));

  EXPECT_FALSE(color0 == color1);
  EXPECT_FALSE(color1 == color0);
  EXPECT_FALSE(color1 == color2);
  EXPECT_FALSE(color2 == color1);

  EXPECT_TRUE(color0 != color1);
  EXPECT_TRUE(color1 != color0);
  EXPECT_TRUE(color1 != color2);
  EXPECT_TRUE(color2 != color1);
}

}  // namespace base
