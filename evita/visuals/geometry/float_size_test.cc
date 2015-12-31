// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/geometry/float_rect.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(FloatSizeTest, Equals) {
  EXPECT_TRUE(FloatSize(0, 0) == FloatSize(0, 0));
  EXPECT_TRUE(FloatSize(1, 2) == FloatSize(1, 2));
  EXPECT_FALSE(FloatSize(0, 0) == FloatSize(0, 1));
  EXPECT_FALSE(FloatSize(0, 0) == FloatSize(1, 0));
}

TEST(FloatSizeTest, IsEmpty) {
  EXPECT_TRUE(FloatSize().IsEmpty());
  EXPECT_TRUE(FloatSize(0, 1).IsEmpty());
  EXPECT_TRUE(FloatSize(1, 0).IsEmpty());
  EXPECT_FALSE(FloatSize(1, 1).IsEmpty());
}

}  // namespace visuals
