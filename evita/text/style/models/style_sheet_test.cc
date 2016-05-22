// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma warning(push)
#pragma warning(disable : 4365 4625 4626 4826)
#include "testing/gtest/include/gtest/gtest.h"
#pragma warning(pop)

#include "base/logging.h"
#include "evita/text/style/models/style.h"
#include "evita/text/style/models/style_resolver.h"
#include "evita/text/style/models/style_selector.h"
#include "evita/text/style/models/style_sheet.h"

namespace {

class StyleSheetTest : public ::testing::Test {
 public:
  StyleSheetTest() = default;
  ~StyleSheetTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(StyleSheetTest);
};

TEST_F(StyleSheetTest, Resolve_default) {
  xcss::StyleSheet style_sheet;
  xcss::StyleResolver style_resolver;
  style_resolver.AddStyleSheet(&style_sheet);
  const auto& style1 = style_resolver.Resolve(xcss::StyleSelector::defaults());
  EXPECT_EQ(xcss::FontStyle::Normal, style1.font_style());
  EXPECT_EQ(xcss::FontWeight::Normal, style1.font_weight());
}

TEST_F(StyleSheetTest, Resolve_some) {
  xcss::StyleSheet style_sheet;
  style_sheet.AddRule(xcss::StyleSelector::active_selection(),
                      xcss::Style(xcss::Color(1, 2, 3), xcss::Color(4, 5, 6)));
  xcss::StyleResolver style_resolver;
  style_resolver.AddStyleSheet(&style_sheet);
  const auto& style1 =
      style_resolver.Resolve(xcss::StyleSelector::active_selection());
  EXPECT_EQ(xcss::Color(1, 2, 3), style1.color());
  EXPECT_EQ(xcss::Color(4, 5, 6), style1.bgcolor());
  EXPECT_EQ(xcss::FontStyle::Normal, style1.font_style());
}

}  // namespace
