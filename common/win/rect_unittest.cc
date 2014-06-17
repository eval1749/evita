// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/win/rect.h"
#include "gtest/gtest.h"

namespace {

using common::win::Rect;

TEST(RectTest, Contains) {
  EXPECT_TRUE(Rect(10, 10, 50, 50).Contains(Rect(10, 10, 50, 50)));
  EXPECT_TRUE(Rect(10, 10, 50, 50).Contains(Rect(11, 11, 49, 49)));
  EXPECT_FALSE(Rect(10, 10, 50, 50).Contains(Rect(60, 60, 70, 70)));
}

}  // namespace
