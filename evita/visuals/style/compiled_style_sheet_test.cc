// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/style/compiled_style_sheet.h"

#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_builder.h"
#include "evita/visuals/css/style_sheet.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_tree_builder.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(CompiledStyleSheetTest, Basic) {
  css::StyleSheet style_sheet;
  style_sheet.AddRule(
      L"tag", css::StyleBuilder().SetColor(1, 0, 0).SetHeight(20).Build());
  style_sheet.AddRule(L".class", css::StyleBuilder().SetColor(0, 1, 0).Build());
  style_sheet.AddRule(L"#id", css::StyleBuilder().SetColor(0, 0, 1).Build());
  CompiledStyleSheet compiled(style_sheet);

  const auto document = NodeTreeBuilder()
                            .Begin(L"tag")
                            .Begin(L"tag", L"id")
                            .End(L"tag")
                            .Begin(L"tag", L"class")
                            .ClassList({L"class"})
                            .End(L"tag")
                            .End(L"tag")
                            .Build();

  const auto tag = document->first_child()->as<Element>();
  const auto tag_class = document->GetElementById(L"class");
  const auto tag_id = document->GetElementById(L"id");

  const auto& tag_style = compiled.Match(*tag);
  const auto& tag_class_style = compiled.Match(*tag_class);
  const auto& tag_id_style = compiled.Match(*tag_id);

  EXPECT_EQ(css::Color(1, 0, 0), tag_style->color());
  EXPECT_EQ(css::Height(css::Length(20)), tag_style->height());

  EXPECT_EQ(css::Color(0, 1, 0), tag_class_style->color());
  EXPECT_EQ(css::Height(css::Length(20)), tag_style->height());

  EXPECT_EQ(css::Color(0, 0, 1), tag_id_style->color());
  EXPECT_EQ(css::Height(css::Length(20)), tag_id_style->height());
}

}  // namespace visuals
