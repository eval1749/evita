// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/base/maybe.h"
#include "gtest/gtest.h"

namespace base {

TEST(Maybe, Basic) {
  auto maybe_one = Just<int>(1);
  EXPECT_TRUE(maybe_one.IsJust());
  EXPECT_FALSE(maybe_one.IsNothing());
  EXPECT_EQ(1, maybe_one.FromJust());
  EXPECT_EQ(1, maybe_one.FromMaybe(2));

  EXPECT_EQ(maybe_one, maybe_one);
  EXPECT_EQ(Just<int>(1), maybe_one);
  EXPECT_NE(Just<int>(2), maybe_one);
  EXPECT_NE(Nothing<int>(), maybe_one);

  auto maybe_two = Nothing<int>();
  EXPECT_FALSE(maybe_two.IsJust());
  EXPECT_TRUE(maybe_two.IsNothing());
  EXPECT_EQ(3, maybe_two.FromMaybe(3));

  EXPECT_NE(maybe_two, maybe_one);
}

TEST(Maybe, Equality) {
  EXPECT_EQ(Nothing<int>(), Nothing<int>());
  EXPECT_NE(Nothing<int>(), Just<int>(1));
  EXPECT_NE(Just<int>(1), Nothing<int>());
  EXPECT_EQ(Just<int>(1), Just<int>(1));
  EXPECT_NE(Just<int>(1), Just<int>(2));
  EXPECT_NE(Just<int>(2), Just<int>(1));
}

TEST(Maybe, Predicate) {
  EXPECT_FALSE(Nothing<int>().IsJust());
  EXPECT_TRUE(Nothing<int>().IsNothing());

  EXPECT_TRUE(Just<int>(1).IsJust());
  EXPECT_FALSE(Just<int>(1).IsNothing());
}

}  // namespace base
