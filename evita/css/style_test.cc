// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/css/style.h"
#include "gtest/gtest.h"

namespace css {

TEST(CssStyle, IsSubsetOf) {
  Style style1;
  Style style2;
  EXPECT_TRUE(style1.IsSubsetOf(style1));
  EXPECT_TRUE(style1.IsSubsetOf(style2));
  EXPECT_TRUE(style2.IsSubsetOf(style1));

  style1.set_color(Color(255, 255, 0));
  EXPECT_FALSE(style1.IsSubsetOf(style2));
  EXPECT_TRUE(style2.IsSubsetOf(style1));

  style2.set_color(Color(255, 255, 0));
  EXPECT_TRUE(style1.IsSubsetOf(style2))
      << "both style1 and style2 has same color property";
  EXPECT_TRUE(style2.IsSubsetOf(style1))
      << "both style1 and style2 has same color property";

  style1.set_bgcolor(Color(255, 255, 255));
  EXPECT_FALSE(style1.IsSubsetOf(style2))
      << "style1 has bgcolor, but style doesn't";
  EXPECT_TRUE(style2.IsSubsetOf(style1)) << "style2 doesn't have bgcolor";
}

}  // namespace css
