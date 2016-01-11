// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/fonts/font_description.h"

#include "evita/visuals/fonts/font_description_builder.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(FontDescriptionTest, Basic) {
  const auto& desc0 = FontDescription();
  const auto& desc1 = FontDescription::Builder().SetSize(10).Build();
  const auto& desc2 = FontDescription::Builder().SetSize(20).Build();

  EXPECT_TRUE(desc0 == desc0);
  EXPECT_FALSE(desc0 == desc1);
  EXPECT_FALSE(desc0 == desc2);

  EXPECT_FALSE(desc0 != desc0);
  EXPECT_TRUE(desc0 != desc1);
  EXPECT_TRUE(desc0 != desc2);

  EXPECT_FALSE(desc1 == desc0);
  EXPECT_TRUE(desc1 == desc1);
  EXPECT_FALSE(desc1 == desc2);

  EXPECT_TRUE(desc1 != desc0);
  EXPECT_FALSE(desc1 != desc1);
  EXPECT_TRUE(desc1 != desc2);

  EXPECT_FALSE(desc2 == desc0);
  EXPECT_FALSE(desc2 == desc1);
  EXPECT_TRUE(desc2 == desc2);

  EXPECT_TRUE(desc2 != desc0);
  EXPECT_TRUE(desc2 != desc1);
  EXPECT_FALSE(desc2 != desc2);
}

}  // namespace visuals
