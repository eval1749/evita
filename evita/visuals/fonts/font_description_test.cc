// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/fonts/font_description.h"

#include "evita/visuals/fonts/font_description_builder.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(FontDescriptionTest, Basic) {
  const auto& desc0 = FontDescription::Builder().Build();
  const auto& desc1 = FontDescription::Builder().SetSize(10).Build();
  const auto& desc2 = FontDescription::Builder().SetSize(20).Build();
  const auto& desc3 = FontDescription::Builder().SetSize(10).Build();

  EXPECT_TRUE(desc0 == desc0);
  EXPECT_FALSE(desc0 == desc1);
  EXPECT_FALSE(desc0 == desc2);
  EXPECT_FALSE(desc0 == desc3);

  EXPECT_FALSE(desc0 != desc0);
  EXPECT_TRUE(desc0 != desc1);
  EXPECT_TRUE(desc0 != desc2);
  EXPECT_TRUE(desc0 != desc3);

  EXPECT_FALSE(desc1 == desc0);
  EXPECT_TRUE(desc1 == desc1);
  EXPECT_FALSE(desc1 == desc2);
  EXPECT_TRUE(desc1 == desc3);

  EXPECT_TRUE(desc1 != desc0);
  EXPECT_FALSE(desc1 != desc1);
  EXPECT_TRUE(desc1 != desc2);
  EXPECT_FALSE(desc1 != desc3);

  EXPECT_FALSE(desc2 == desc0);
  EXPECT_FALSE(desc2 == desc1);
  EXPECT_TRUE(desc2 == desc2);
  EXPECT_FALSE(desc2 == desc3);

  EXPECT_TRUE(desc2 != desc0);
  EXPECT_TRUE(desc2 != desc1);
  EXPECT_FALSE(desc2 != desc2);
  EXPECT_TRUE(desc2 != desc3);

  EXPECT_EQ(&desc1, &desc3);
}

TEST(FontDescriptionTest, Build) {
  const auto& desc = FontDescription::Builder()
                         .SetFamily(L"Arial")
                         .SetSize(42)
                         .SetStretch(FontStretch::Condensed)
                         .SetStyle(FontStyle::Italic)
                         .SetWeight(FontWeight::Bold)
                         .Build();
  EXPECT_EQ(L"Arial", desc.family());
  EXPECT_EQ(42.0f, desc.size());
  EXPECT_EQ(FontStretch::Condensed, desc.stretch());
  EXPECT_EQ(FontStyle::Italic, desc.style());
  EXPECT_EQ(FontWeight::Bold, desc.weight());
}

}  // namespace visuals
