// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>
#include <unordered_map>
#include <utility>

#include "evita/base/strings/atomic_string.h"
#include "evita/base/strings/atomic_string_factory.h"
#include "gtest/gtest.h"

namespace evita {

TEST(AtomicStringTest, Map) {
  std::map<AtomicString, int> map;
  map.emplace(AtomicString(L"one"), 1);
  map.emplace(AtomicString(L"two"), 2);

  EXPECT_EQ(2, map.size());
  EXPECT_EQ(AtomicString(L"one"), map.find(AtomicString(L"one"))->first);
  EXPECT_EQ(1, map.find(AtomicString(L"one"))->second);
  EXPECT_EQ(AtomicString(L"two"), map.find(AtomicString(L"two"))->first);
  EXPECT_EQ(2, map.find(AtomicString(L"two"))->second);
  EXPECT_TRUE(map.find(AtomicString(L"foo")) == map.end());
  const auto inserted = map.insert(std::make_pair(AtomicString(L"one"), -1));
  EXPECT_FALSE(inserted.second) << "should not be inserted";
  EXPECT_EQ(AtomicString(L"one"), inserted.first->first);
  EXPECT_EQ(1, inserted.first->second);
}

TEST(AtomicStringTest, NewAtomicString) {
  const auto& name1 = AtomicString(L"foo");
  const auto& name2 = AtomicString(L"foo");
  EXPECT_EQ(name1, name2);
  EXPECT_EQ(L"foo", name1.value().as_string());
}

TEST(AtomicStringTest, NewUniqueAtomicString) {
  const auto& name1 = AtomicString::NewUniqueString(L"foo%d");
  const auto& name2 = AtomicString::NewUniqueString(L"foo%d");
  EXPECT_NE(name1, name2);
}

TEST(AtomicStringTest, UnorderedMap) {
  std::unordered_map<AtomicString, int> map;
  map.emplace(AtomicString(L"one"), 1);
  map.emplace(AtomicString(L"two"), 2);

  EXPECT_EQ(2, map.size());
  EXPECT_EQ(AtomicString(L"one"), map.find(AtomicString(L"one"))->first);
  EXPECT_EQ(1, map.find(AtomicString(L"one"))->second);
  EXPECT_EQ(AtomicString(L"two"), map.find(AtomicString(L"two"))->first);
  EXPECT_EQ(2, map.find(AtomicString(L"two"))->second);
  EXPECT_TRUE(map.find(AtomicString(L"foo")) == map.end());
  const auto inserted = map.insert(std::make_pair(AtomicString(L"one"), -1));
  EXPECT_FALSE(inserted.second) << "should not be inserted";
  EXPECT_EQ(AtomicString(L"one"), inserted.first->first);
  EXPECT_EQ(1, inserted.first->second);
}

}  // namespace evita
