// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/css/property_set.h"

#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/property_set_builder.h"
#include "evita/visuals/css/value.h"
#include "evita/visuals/css/values.h"
#include "gtest/gtest.h"

namespace visuals {
namespace css {

TEST(PropertySetTest, Basic) {
  PropertySet set1;
  PropertySet set2 =
      PropertySet::Builder().AddInteger(PropertyId::Width, 100).Build();
  PropertySet set3;

  EXPECT_EQ(set1, set1);
  EXPECT_NE(set1, set2);
  EXPECT_EQ(set1, set3);

  EXPECT_NE(set2, set1);
  EXPECT_EQ(set2, set2);
  EXPECT_NE(set2, set3);

  EXPECT_EQ(set3, set1);
  EXPECT_NE(set3, set2);
  EXPECT_EQ(set3, set3);
}

TEST(PropertySetTest, Color) {
  PropertySet set1 =
      PropertySet::Builder()
          .AddColor(PropertyId::Color, Color::Rgba(128, 192, 128))
          .AddColor(PropertyId::BorderTopColor,
                    Color::Rgba(192, 192, 128, 0.5f))
          .Build();
  EXPECT_EQ(Value(Color::Rgba(128, 192, 128)), set1.ValueOf(PropertyId::Color));
  EXPECT_EQ(Value(Color::Rgba(192, 192, 128, 0.5f)),
            set1.ValueOf(PropertyId::BorderTopColor));
}

TEST(PropertySetTest, Dimension) {
  PropertySet set1 =
      PropertySet::Builder()
          .AddDimension(PropertyId::Height, Dimension(1.5f, Unit::cm))
          .AddDimension(PropertyId::Width, Dimension(1.23456f, Unit::in))
          .Build();
  EXPECT_EQ(Value(Dimension(1.5f, Unit::cm)), set1.ValueOf(PropertyId::Height));
  EXPECT_EQ(Value(Dimension(1.23456f, Unit::in)),
            set1.ValueOf(PropertyId::Width));
}

TEST(PropertySetTest, Equals) {
  PropertySet set1 = PropertySet::Builder()
                         .AddInteger(PropertyId::Height, 123)
                         .AddInteger(PropertyId::Width, 456)
                         .Build();
  PropertySet set2 = PropertySet::Builder()
                         .AddInteger(PropertyId::Width, 456)
                         .AddInteger(PropertyId::Height, 123)
                         .Build();
  EXPECT_TRUE(set1 == set2)
      << "Order of properties doesn't matter for equality.";
}

TEST(PropertySetTest, Integer) {
  PropertySet set1 = PropertySet::Builder()
                         .AddInteger(PropertyId::Height, 100)
                         .AddInteger(PropertyId::Width, 1 << 30)
                         .Build();
  EXPECT_EQ(Value(100), set1.ValueOf(PropertyId::Height));
  EXPECT_EQ(Value(1 << 30), set1.ValueOf(PropertyId::Width));
}

TEST(PropertySetTest, Keyword) {
  PropertySet set1 = PropertySet::Builder()
                         .AddKeyword(PropertyId::Height, Keyword::Auto)
                         .AddKeyword(PropertyId::Width, Keyword::Inherit)
                         .Build();
  EXPECT_EQ(Value(Keyword::Auto), set1.ValueOf(PropertyId::Height));
  EXPECT_TRUE(set1.ValueOf(PropertyId::Height).is_auto());
  EXPECT_EQ(Value(Keyword::Inherit), set1.ValueOf(PropertyId::Width));
  EXPECT_TRUE(set1.ValueOf(PropertyId::Width).is_inherit());
}

TEST(PropertySetTest, Number) {
  PropertySet set1 = PropertySet::Builder()
                         .AddNumber(PropertyId::Height, 1.5f)
                         .AddNumber(PropertyId::Width, 1.23456f)
                         .Build();
  EXPECT_EQ(Value(1.5f), set1.ValueOf(PropertyId::Height));
  EXPECT_EQ(Value(1.23456f), set1.ValueOf(PropertyId::Width));
}

TEST(PropertySetTest, Percentage) {
  PropertySet set1 = PropertySet::Builder()
                         .AddPercentage(PropertyId::Height, 100)
                         .AddPercentage(PropertyId::Width, 1.23456f)
                         .Build();
  EXPECT_EQ(Value(Percentage(100)), set1.ValueOf(PropertyId::Height));
  EXPECT_EQ(Value(Percentage(1.23456f)), set1.ValueOf(PropertyId::Width));
}

}  // namespace css
}  // namespace visuals
