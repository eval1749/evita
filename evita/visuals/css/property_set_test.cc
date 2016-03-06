// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/css/property_set.h"

#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/property_set_builder.h"
#include "evita/visuals/css/property_set_editor.h"
#include "evita/visuals/css/values.h"
#include "evita/visuals/css/values/value.h"
#include "gtest/gtest.h"

namespace visuals {
namespace css {

class PropertySetTest : public ::testing::Test {
 protected:
  PropertySetTest() = default;
  ~PropertySetTest() override = default;

  size_t SizeOfWords(const PropertySet& property_set) const {
    return property_set.words_.size();
  }
};

TEST_F(PropertySetTest, Basic) {
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

TEST_F(PropertySetTest, Color) {
  PropertySet set1 =
      PropertySet::Builder()
          .AddColor(PropertyId::Color, ColorValue::Rgba(128, 192, 128))
          .AddColor(PropertyId::BorderTopColor,
                    ColorValue::Rgba(192, 192, 128, 0.5f))
          .Build();
  EXPECT_EQ(Value(ColorValue::Rgba(128, 192, 128)),
            set1.ValueOf(PropertyId::Color));
  EXPECT_EQ(Value(ColorValue::Rgba(192, 192, 128, 0.5f)),
            set1.ValueOf(PropertyId::BorderTopColor));
}

TEST_F(PropertySetTest, Contains) {
  PropertySet set1 =
      PropertySet::Builder()
          .AddColor(PropertyId::Color, ColorValue::Rgba(128, 192, 128))
          .AddColor(PropertyId::BorderTopColor,
                    ColorValue::Rgba(192, 192, 128, 0.5f))
          .Build();
  EXPECT_TRUE(set1.Contains(PropertyId::Color));
  EXPECT_TRUE(set1.Contains(PropertyId::BorderTopColor));
  EXPECT_FALSE(set1.Contains(PropertyId::Display));
}

TEST_F(PropertySetTest, Dimension) {
  PropertySet set1 =
      PropertySet::Builder()
          .AddDimension(PropertyId::Height, Dimension(1.5f, Unit::cm))
          .AddDimension(PropertyId::Width, Dimension(1.23456f, Unit::in))
          .Build();
  EXPECT_EQ(Value(Dimension(1.5f, Unit::cm)), set1.ValueOf(PropertyId::Height));
  EXPECT_EQ(Value(Dimension(1.23456f, Unit::in)),
            set1.ValueOf(PropertyId::Width));
}

TEST_F(PropertySetTest, Editor) {
  PropertySet set1 =
      PropertySet::Builder()
          .AddColor(PropertyId::Color, ColorValue::Rgba(128, 192, 128))
          .AddColor(PropertyId::BorderTopColor,
                    ColorValue::Rgba(192, 192, 128, 0.5f))
          .Build();

  PropertySet::Editor().SetColor(&set1, Value(ColorValue::Rgba(192, 192, 128)));
  EXPECT_EQ(4, SizeOfWords(set1));
  EXPECT_EQ(Value(ColorValue::Rgba(192, 192, 128)),
            set1.ValueOf(PropertyId::Color));

  PropertySet::Editor().SetDisplay(&set1, Value(Keyword::None));
  EXPECT_TRUE(set1.Contains(PropertyId::Display));
  EXPECT_EQ(5, SizeOfWords(set1));

  PropertySet::Editor().SetDisplay(&set1, Value(Keyword::Inline));
  EXPECT_EQ(5, SizeOfWords(set1));

  PropertySet::Editor().RemoveColor(&set1);
  EXPECT_EQ(3, SizeOfWords(set1));
  EXPECT_FALSE(set1.Contains(PropertyId::Color));

  PropertySet::Editor().SetWidth(&set1, Value(Dimension(1.5f, Unit::px)));
  EXPECT_EQ(4, SizeOfWords(set1));

  PropertySet::Editor().SetWidth(&set1, Value(Dimension(123.456f, Unit::px)));
  EXPECT_EQ(5, SizeOfWords(set1)) << "small dimension to normal dimension";

  PropertySet::Editor().SetWidth(&set1, Value(Dimension(0.0f, Unit::px)));
  EXPECT_EQ(4, SizeOfWords(set1)) << "normal dimension to small dimension";
}

TEST_F(PropertySetTest, Equals) {
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

TEST_F(PropertySetTest, Integer) {
  PropertySet set1 = PropertySet::Builder()
                         .AddInteger(PropertyId::Height, 100)
                         .AddInteger(PropertyId::Width, 1 << 30)
                         .Build();
  EXPECT_EQ(Value(100), set1.ValueOf(PropertyId::Height));
  EXPECT_EQ(Value(1 << 30), set1.ValueOf(PropertyId::Width));
}

TEST_F(PropertySetTest, Keyword) {
  PropertySet set1 = PropertySet::Builder()
                         .AddKeyword(PropertyId::Height, Keyword::Auto)
                         .AddKeyword(PropertyId::Width, Keyword::Inherit)
                         .Build();
  EXPECT_EQ(Value(Keyword::Auto), set1.ValueOf(PropertyId::Height));
  EXPECT_TRUE(set1.ValueOf(PropertyId::Height).is_auto());
  EXPECT_EQ(Value(Keyword::Inherit), set1.ValueOf(PropertyId::Width));
  EXPECT_TRUE(set1.ValueOf(PropertyId::Width).is_inherit());
}

TEST_F(PropertySetTest, Number) {
  PropertySet set1 = PropertySet::Builder()
                         .AddNumber(PropertyId::Height, 1.5f)
                         .AddNumber(PropertyId::Width, 1.23456f)
                         .Build();
  EXPECT_EQ(Value(1.5f), set1.ValueOf(PropertyId::Height));
  EXPECT_EQ(Value(1.23456f), set1.ValueOf(PropertyId::Width));
}

TEST_F(PropertySetTest, Percentage) {
  PropertySet set1 = PropertySet::Builder()
                         .AddPercentage(PropertyId::Height, 100)
                         .AddPercentage(PropertyId::Width, 1.23456f)
                         .Build();
  EXPECT_EQ(Value(Percentage(100)), set1.ValueOf(PropertyId::Height));
  EXPECT_EQ(Value(Percentage(1.23456f)), set1.ValueOf(PropertyId::Width));
}

TEST_F(PropertySetTest, String) {
  PropertySet set1 =
      PropertySet::Builder().AddString(PropertyId::FontFamily, L"foo").Build();
  EXPECT_EQ(Value(String(L"foo")), set1.ValueOf(PropertyId::FontFamily));
}

}  // namespace css
}  // namespace visuals
