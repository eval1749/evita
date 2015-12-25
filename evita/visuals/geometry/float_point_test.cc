// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/geometry/float_rect.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(FloatPointTest, Equals) {
  EXPECT_TRUE(FloatPoint(0, 0) == FloatPoint(0, 0));
  EXPECT_TRUE(FloatPoint(1, 2) == FloatPoint(1, 2));
  EXPECT_FALSE(FloatPoint(0, 0) == FloatPoint(0, 1));
  EXPECT_FALSE(FloatPoint(0, 0) == FloatPoint(1, 0));
}

}  // namespace visuals
