// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "joana/base/hash_utils.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace joana {

namespace {

template <typename T>
size_t ComputeHash(const T& value) {
  return std::hash<T>{}(value);
}

}  // namespace

TEST(HashUtlsTest, HashPair) {
  EXPECT_EQ(ComputeHash(std::make_pair(1, 2)),
            ComputeHash(std::make_pair(1, 2)));

  EXPECT_NE(ComputeHash(std::make_pair(2, 1)),
            ComputeHash(std::make_pair(1, 2)))
      << "Hash values of <1, 2> and <2, 1> should be different";
}

TEST(HashUtlsTest, HashTuple) {
  EXPECT_EQ(0, ComputeHash(std::make_tuple()));

  EXPECT_EQ(ComputeHash(std::make_tuple(1)), ComputeHash(std::make_tuple(1)));

  EXPECT_EQ(ComputeHash(std::make_tuple(1, 2)),
            ComputeHash(std::make_tuple(1, 2)));

  EXPECT_EQ(ComputeHash(std::make_tuple(1, 2, 3)),
            ComputeHash(std::make_tuple(1, 2, 3)))
      << "ComputeHash(<1, 2, 3>) should return same value always";

  EXPECT_NE(ComputeHash(std::make_tuple(2, 1)),
            ComputeHash(std::make_tuple(1, 2)))
      << "Hash values of <1, 2> and <2, 1> should be different";

  EXPECT_NE(ComputeHash(std::make_tuple(3, 2, 1)),
            ComputeHash(std::make_tuple(1, 2, 3)))
      << "Hash values of <1, 2, 3> and <3, 2, 1> should be different";

  EXPECT_EQ(ComputeHash(std::make_tuple(1, base::StringPiece("foo"))),
            ComputeHash(std::make_tuple(1, base::StringPiece("foo"))));
}

}  // namespace joana
