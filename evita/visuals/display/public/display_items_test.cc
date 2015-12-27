// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/display/public/display_items.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(BeginBoxDisplayItemsTest, EqualsTo) {
  const auto& item1 =
      std::make_unique<BeginBoxDisplayItem>(1, FloatRect(FloatSize(1, 2)));
  const auto& item2 =
      std::make_unique<BeginBoxDisplayItem>(1, FloatRect(FloatSize(1, 2)));
  const auto& item3 =
      std::make_unique<BeginBoxDisplayItem>(1, FloatRect(FloatSize(3, 4)));
  const auto& item4 =
      std::make_unique<BeginBoxDisplayItem>(2, FloatRect(FloatSize(1, 2)));

  EXPECT_TRUE(*item1 == *item1);
  EXPECT_TRUE(*item2 == *item2);

  EXPECT_TRUE(*item1 == *item2);
  EXPECT_TRUE(*item2 == *item1);

  EXPECT_FALSE(*item1 != *item2);
  EXPECT_FALSE(*item2 != *item1);

  EXPECT_FALSE(*item1 == *item3);
  EXPECT_FALSE(*item3 == *item1);
  EXPECT_TRUE(*item1 != *item3);
  EXPECT_TRUE(*item3 != *item1);

  EXPECT_FALSE(*item1 == *item4);
  EXPECT_FALSE(*item4 == *item1);
  EXPECT_TRUE(*item1 != *item4);
  EXPECT_TRUE(*item4 != *item1);
}

TEST(DrawRectDisplayItemsTest, EqualsTo) {
  const auto& item1 = std::make_unique<DrawRectDisplayItem>(
      FloatRect(FloatSize(1, 2)), FloatColor(1, 1, 1), 1);
  const auto& item2 = std::make_unique<DrawRectDisplayItem>(
      FloatRect(FloatSize(1, 2)), FloatColor(1, 1, 1), 1);
  const auto& item3 = std::make_unique<DrawRectDisplayItem>(
      FloatRect(FloatSize(3, 4)), FloatColor(1, 1, 1), 1);
  const auto& item4 = std::make_unique<DrawRectDisplayItem>(
      FloatRect(FloatSize(1, 2)), FloatColor(0.5, 1, 1), 1);
  const auto& item5 = std::make_unique<DrawRectDisplayItem>(
      FloatRect(FloatSize(1, 2)), FloatColor(1, 1, 1), 2);

  EXPECT_TRUE(*item1 == *item1);
  EXPECT_TRUE(*item1 == *item2);
  EXPECT_FALSE(*item1 == *item3);
  EXPECT_FALSE(*item1 == *item4);
  EXPECT_FALSE(*item1 == *item5);

  EXPECT_FALSE(*item1 != *item1);
  EXPECT_FALSE(*item1 != *item2);
  EXPECT_TRUE(*item1 != *item3);
  EXPECT_TRUE(*item1 != *item4);
  EXPECT_TRUE(*item1 != *item5);

  EXPECT_TRUE(*item2 == *item1);

  EXPECT_FALSE(*item3 == *item1);
  EXPECT_TRUE(*item3 != *item1);

  EXPECT_FALSE(*item4 == *item1);
  EXPECT_TRUE(*item4 != *item1);

  EXPECT_FALSE(*item5 == *item1);
  EXPECT_TRUE(*item5 != *item1);
}

TEST(DrawTextDisplayItemsTest, EqualsTo) {
  const auto& item1 = std::make_unique<DrawTextDisplayItem>(
      FloatRect(FloatSize(1, 2)), FloatColor(1, 1, 1), 1, L"foo");
  const auto& item2 = std::make_unique<DrawTextDisplayItem>(
      FloatRect(FloatSize(1, 2)), FloatColor(1, 1, 1), 1, L"foo");
  const auto& item3 = std::make_unique<DrawTextDisplayItem>(
      FloatRect(FloatSize(3, 4)), FloatColor(1, 1, 1), 1, L"foo");
  const auto& item4 = std::make_unique<DrawTextDisplayItem>(
      FloatRect(FloatSize(1, 2)), FloatColor(0.5, 1, 1), 1, L"foo");
  const auto& item5 = std::make_unique<DrawTextDisplayItem>(
      FloatRect(FloatSize(1, 2)), FloatColor(1, 1, 1), 2, L"foo");
  const auto& item6 = std::make_unique<DrawTextDisplayItem>(
      FloatRect(FloatSize(1, 2)), FloatColor(1, 1, 1), 1, L"bar");

  EXPECT_TRUE(*item1 == *item1);
  EXPECT_TRUE(*item1 == *item2);
  EXPECT_FALSE(*item1 == *item3);
  EXPECT_FALSE(*item1 == *item4);
  EXPECT_FALSE(*item1 == *item5);
  EXPECT_FALSE(*item1 == *item6);

  EXPECT_FALSE(*item1 != *item1);
  EXPECT_FALSE(*item1 != *item2);
  EXPECT_TRUE(*item1 != *item3);
  EXPECT_TRUE(*item1 != *item4);
  EXPECT_TRUE(*item1 != *item5);
  EXPECT_TRUE(*item1 != *item6);

  EXPECT_TRUE(*item2 == *item1);

  EXPECT_FALSE(*item3 == *item1);
  EXPECT_TRUE(*item3 != *item1);

  EXPECT_FALSE(*item4 == *item1);
  EXPECT_TRUE(*item4 != *item1);

  EXPECT_FALSE(*item5 == *item1);
  EXPECT_TRUE(*item5 != *item1);

  EXPECT_FALSE(*item6 == *item1);
  EXPECT_TRUE(*item6 != *item1);
}

TEST(EndBoxDisplayItemsTest, EqualsTo) {
  const auto& item1 = std::make_unique<EndBoxDisplayItem>(1);
  const auto& item2 = std::make_unique<EndBoxDisplayItem>(1);
  const auto& item3 = std::make_unique<EndBoxDisplayItem>(2);

  EXPECT_TRUE(*item1 == *item1);
  EXPECT_TRUE(*item1 == *item2);
  EXPECT_FALSE(*item1 == *item3);

  EXPECT_FALSE(*item1 != *item1);
  EXPECT_FALSE(*item1 != *item2);
  EXPECT_TRUE(*item1 != *item3);

  EXPECT_TRUE(*item2 == *item1);
  EXPECT_FALSE(*item2 != *item1);

  EXPECT_FALSE(*item3 == *item1);
  EXPECT_TRUE(*item3 != *item1);
}

TEST(FillRectDisplayItemsTest, EqualsTo) {
  const auto& item1 = std::make_unique<FillRectDisplayItem>(
      FloatRect(FloatSize(1, 2)), FloatColor(1, 1, 1));
  const auto& item2 = std::make_unique<FillRectDisplayItem>(
      FloatRect(FloatSize(1, 2)), FloatColor(1, 1, 1));
  const auto& item3 = std::make_unique<FillRectDisplayItem>(
      FloatRect(FloatSize(1, 2)), FloatColor(0.5, 1, 1));
  const auto& item4 = std::make_unique<FillRectDisplayItem>(
      FloatRect(FloatSize(2, 2)), FloatColor(1, 1, 1));

  EXPECT_TRUE(*item1 == *item1);
  EXPECT_TRUE(*item1 == *item2);
  EXPECT_FALSE(*item1 == *item3);
  EXPECT_FALSE(*item1 == *item4);

  EXPECT_FALSE(*item1 != *item1);
  EXPECT_FALSE(*item1 != *item2);
  EXPECT_TRUE(*item1 != *item3);
  EXPECT_TRUE(*item1 != *item4);

  EXPECT_TRUE(*item2 == *item1);

  EXPECT_FALSE(*item3 == *item1);
  EXPECT_TRUE(*item3 != *item1);

  EXPECT_FALSE(*item4 == *item1);
  EXPECT_TRUE(*item4 != *item1);
}

}  // namespace visuals
