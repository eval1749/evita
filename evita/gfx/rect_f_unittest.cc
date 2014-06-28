// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/gfx/rect_f.h"
#include "gtest/gtest.h"

namespace {

using gfx::RectF;

TEST(RectFTest, Contains) {
  EXPECT_TRUE(RectF(10, 10, 50, 50).Contains(RectF(10, 10, 50, 50)));
  EXPECT_TRUE(RectF(10, 10, 50, 50).Contains(RectF(11, 11, 49, 49)));
  EXPECT_FALSE(RectF(10, 10, 50, 50).Contains(RectF(60, 60, 70, 70)));
}

TEST(RectFTest, Intersect) {
  EXPECT_EQ(RectF(10, 20, 30, 40),
            RectF(10, 20, 30, 40).Intersect(RectF(0, 0, 100, 100)));
  EXPECT_TRUE(RectF(10, 20, 30, 40).Intersect(RectF(0, 0, 1, 1)).empty());
}

}  // namespace
