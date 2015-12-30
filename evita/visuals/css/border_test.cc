// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/css/border.h"
#include "gtest/gtest.h"

namespace visuals {
namespace css {

TEST(BorderTest, Equals) {
  const auto& border0 = Border();
  const auto& border1 = Border(Color(1, 1, 1), 1.0);
  const auto& border2 = Border(Color(1, 1, 1), 1.0, 1.0, 1.0, 1.0);
  const auto& border3 = Border(Color(1, 1, 1), 2.0, 1.0, 1.0, 2.0);

  EXPECT_TRUE(border0 == border0);
  EXPECT_FALSE(border0 != border0);
  EXPECT_FALSE(border0 == border1);
  EXPECT_FALSE(border0 == border2);
  EXPECT_FALSE(border0 == border3);
  EXPECT_TRUE(border0 != border1);
  EXPECT_TRUE(border0 != border2);
  EXPECT_TRUE(border0 != border3);

  EXPECT_TRUE(border1 == border1);
  EXPECT_TRUE(border1 == border2);
  EXPECT_FALSE(border1 != border1);
  EXPECT_FALSE(border1 != border2);

  EXPECT_FALSE(border2 == border0);
  EXPECT_TRUE(border2 == border1);
  EXPECT_TRUE(border2 == border2);
  EXPECT_FALSE(border2 == border3);
  EXPECT_TRUE(border2 != border0);
  EXPECT_FALSE(border2 != border1);
  EXPECT_FALSE(border2 != border2);
  EXPECT_TRUE(border2 != border3);
}

}  // namespace css
}  // namespace visuals
