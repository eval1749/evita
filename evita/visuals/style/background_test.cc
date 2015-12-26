// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/style/background.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(BackgroundTest, Equals) {
  Background background0;
  Background background1(FloatColor(1, 2, 3, 4));
  Background background2(FloatColor(5, 6, 7, 8));

  EXPECT_TRUE(background0 == background0);
  EXPECT_TRUE(background1 == background1);
  EXPECT_TRUE(background2 == background2);

  EXPECT_FALSE(background0 != background0);
  EXPECT_FALSE(background1 != background1);
  EXPECT_FALSE(background2 != background2);

  EXPECT_TRUE(background0 == Background());
  EXPECT_TRUE(background1 == Background(FloatColor(1, 2, 3, 4)));
  EXPECT_TRUE(background2 == Background(FloatColor(5, 6, 7, 8)));

  EXPECT_FALSE(background0 != Background());
  EXPECT_FALSE(background1 != Background(FloatColor(1, 2, 3, 4)));
  EXPECT_FALSE(background2 != Background(FloatColor(5, 6, 7, 8)));

  EXPECT_FALSE(background0 == background1);
  EXPECT_FALSE(background1 == background0);
  EXPECT_FALSE(background1 == background2);
  EXPECT_FALSE(background2 == background1);

  EXPECT_TRUE(background0 != background1);
  EXPECT_TRUE(background1 != background0);
  EXPECT_TRUE(background1 != background2);
  EXPECT_TRUE(background2 != background1);
}

}  // namespace visuals
