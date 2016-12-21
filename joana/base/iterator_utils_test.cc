// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "joana/base/iterator_utils.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace joana {

namespace {

struct Thing {
  int value;

  explicit Thing(int init) : value(init) {}
};

int MyFunc(const Thing& thing) {
  return thing.value;
}

}  // namespace

TEST(IteratorUtilsTest, Const) {
  std::vector<const Thing*> vector{new Thing(1), new Thing(2), new Thing(3)};
  auto sum = 0;
  for (auto& thing : ReferenceRangeOf(vector))
    sum += MyFunc(thing);
  EXPECT_EQ(6, sum);
  EXPECT_EQ(3u, ReferenceRangeOf(vector).size());
}

TEST(IteratorUtilsTest, NonConst) {
  std::vector<Thing*> vector{new Thing(1), new Thing(2), new Thing(3)};
  auto sum = 0;
  for (auto& thing : ReferenceRangeOf(vector)) {
    thing.value *= 2;
    sum += MyFunc(thing);
  }
  EXPECT_EQ(12, sum);
  EXPECT_EQ(3u, ReferenceRangeOf(vector).size());
}

}  // namespace joana
