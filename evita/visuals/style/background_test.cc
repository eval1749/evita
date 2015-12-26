// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/style/background.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(BackgroundTest, Equals) {
  Background background0;
  Background background1(FloatColor(0.1f, 0.2f, 0.3f, 0.4f));
  Background background2(FloatColor(0.5f, 0.6f, 0.7f, 0.8f));

  EXPECT_TRUE(background0 == background0);
  EXPECT_TRUE(background1 == background1);
  EXPECT_TRUE(background2 == background2);

  EXPECT_FALSE(background0 != background0);
  EXPECT_FALSE(background1 != background1);
  EXPECT_FALSE(background2 != background2);

  EXPECT_TRUE(background0 == Background());
  EXPECT_TRUE(background1 == Background(FloatColor(0.1f, 0.2f, 0.3f, 0.4f)));
  EXPECT_TRUE(background2 == Background(FloatColor(0.5f, 0.6f, 0.7f, 0.8f)));

  EXPECT_FALSE(background0 != Background());
  EXPECT_FALSE(background1 != Background(FloatColor(0.1f, 0.2f, 0.3f, 0.4f)));
  EXPECT_FALSE(background2 != Background(FloatColor(0.5f, 0.6f, 0.7f, 0.8f)));

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
