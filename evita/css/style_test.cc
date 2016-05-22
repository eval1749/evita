// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/css/style.h"
#include "evita/css/style_builder.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace css {

TEST(StyleTest, color) {
  const auto& style1 = StyleBuilder().SetColor(ColorValue(1, 0, 0)).Build();

  EXPECT_TRUE(style1->has_color());
  EXPECT_EQ(Color(ColorValue(1, 0, 0)), style1->color());
}

TEST(StyleTest, has) {
  const auto& style1 = StyleBuilder().Build();
  EXPECT_FALSE(style1->has_color());
}

TEST(StyleTest, Equals) {
  const auto& style1 = StyleBuilder().Build();
  const auto& style2 = StyleBuilder().SetColor(ColorValue(1, 0, 0)).Build();
  const auto& style3 = StyleBuilder().SetColor(ColorValue(1, 0, 0)).Build();

  EXPECT_TRUE(*style1 == *style1);
  EXPECT_FALSE(*style1 != *style1);
  EXPECT_TRUE(*style2 == *style2);
  EXPECT_FALSE(*style2 != *style2);

  EXPECT_FALSE(*style1 == *style2);
  EXPECT_FALSE(*style2 == *style1);
  EXPECT_TRUE(*style1 != *style2);
  EXPECT_TRUE(*style2 != *style1);

  EXPECT_TRUE(*style2 == *style3);
  EXPECT_TRUE(*style3 == *style2);
  EXPECT_FALSE(*style2 != *style3);
  EXPECT_FALSE(*style3 != *style2);
}

}  // namespace css
