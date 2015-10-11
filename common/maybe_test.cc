// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "common/maybe.h"
#include "gtest/gtest.h"

TEST(Maybe, Basic) {
  auto maybe_one = common::Just<int>(1);
  EXPECT_TRUE(maybe_one.IsJust());
  EXPECT_FALSE(maybe_one.IsNothing());
  EXPECT_EQ(1, maybe_one.FromJust());
  EXPECT_EQ(1, maybe_one.FromMaybe(2));

  EXPECT_EQ(maybe_one, maybe_one);
  EXPECT_EQ(common::Just<int>(1), maybe_one);
  EXPECT_NE(common::Just<int>(2), maybe_one);
  EXPECT_NE(common::Nothing<int>(), maybe_one);

  auto maybe_two = common::Nothing<int>();
  EXPECT_FALSE(maybe_two.IsJust());
  EXPECT_TRUE(maybe_two.IsNothing());
  EXPECT_EQ(3, maybe_two.FromMaybe(3));

  EXPECT_NE(maybe_two, maybe_one);
}

TEST(Maybe, Equality) {
  EXPECT_EQ(common::Nothing<int>(), common::Nothing<int>());
  EXPECT_NE(common::Nothing<int>(), common::Just<int>(1));
  EXPECT_NE(common::Just<int>(1), common::Nothing<int>());
  EXPECT_EQ(common::Just<int>(1), common::Just<int>(1));
  EXPECT_NE(common::Just<int>(1), common::Just<int>(2));
  EXPECT_NE(common::Just<int>(2), common::Just<int>(1));
}

TEST(Maybe, Predicate) {
  EXPECT_FALSE(common::Nothing<int>().IsJust());
  EXPECT_TRUE(common::Nothing<int>().IsNothing());

  EXPECT_TRUE(common::Just<int>(1).IsJust());
  EXPECT_FALSE(common::Just<int>(1).IsNothing());
}
