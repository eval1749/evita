// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/style/color.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(FloatColorTest, Equals) {
  FloatColor color0;
  FloatColor color1(0.1f, 0.2f, 0.3f, 0.4f);
  FloatColor color2(0.5f, 0.6f, 0.7f, 0.8f);

  EXPECT_TRUE(color0 == color0);
  EXPECT_TRUE(color1 == color1);
  EXPECT_TRUE(color2 == color2);

  EXPECT_FALSE(color0 != color0);
  EXPECT_FALSE(color1 != color1);
  EXPECT_FALSE(color2 != color2);

  EXPECT_TRUE(color0 == FloatColor());
  EXPECT_TRUE(color1 == FloatColor(0.1f, 0.2f, 0.3f, 0.4f));
  EXPECT_TRUE(color2 == FloatColor(0.5f, 0.6f, 0.7f, 0.8f));

  EXPECT_FALSE(color0 != FloatColor());
  EXPECT_FALSE(color1 != FloatColor(0.1f, 0.2f, 0.3f, 0.4f));
  EXPECT_FALSE(color2 != FloatColor(0.5f, 0.6f, 0.7f, 0.8f));

  EXPECT_FALSE(color0 == color1);
  EXPECT_FALSE(color1 == color0);
  EXPECT_FALSE(color1 == color2);
  EXPECT_FALSE(color2 == color1);

  EXPECT_TRUE(color0 != color1);
  EXPECT_TRUE(color1 != color0);
  EXPECT_TRUE(color1 != color2);
  EXPECT_TRUE(color2 != color1);
}

}  // namespace visuals
