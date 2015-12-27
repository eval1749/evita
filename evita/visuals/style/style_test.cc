// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/style/style.h"
#include "evita/visuals/style/style_builder.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(StyleTest, color) {
  const auto& style1 = StyleBuilder().SetColor(FloatColor(1, 0, 0)).Build();

  EXPECT_TRUE(style1->has_color());
  EXPECT_EQ(FloatColor(1, 0, 0), style1->color());
}

TEST(StyleTest, has) {
  const auto& style1 = StyleBuilder().Build();
  EXPECT_FALSE(style1->has_color());
}

TEST(StyleTest, Equals) {
  const auto& style1 = StyleBuilder().Build();
  const auto& style2 = StyleBuilder().SetColor(FloatColor(1, 0, 0)).Build();
  const auto& style3 = StyleBuilder().SetColor(FloatColor(1, 0, 0)).Build();

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

}  // namespace visuals
