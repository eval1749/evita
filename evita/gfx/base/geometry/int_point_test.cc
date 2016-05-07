// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/base/geometry/int_point.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace gfx {

TEST(IntPointTest, Equals) {
  EXPECT_TRUE(IntPoint(0, 0) == IntPoint(0, 0));
  EXPECT_TRUE(IntPoint(1, 2) == IntPoint(1, 2));
  EXPECT_FALSE(IntPoint(0, 0) == IntPoint(0, 1));
  EXPECT_FALSE(IntPoint(0, 0) == IntPoint(1, 0));
}

}  // namespace gfx
