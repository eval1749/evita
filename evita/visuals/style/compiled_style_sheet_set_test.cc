// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/style/compiled_style_sheet_set.h"

#include "evita/css/properties.h"
#include "evita/css/selector_builder.h"
#include "evita/css/selector_parser.h"
#include "evita/css/style.h"
#include "evita/css/style_builder.h"
#include "evita/css/style_sheet.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_tree_builder.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace visuals {

namespace {

std::unique_ptr<css::Style> ComputeStyle(
    const CompiledStyleSheetSet& style_sheet,
    const ElementNode& element) {
  css::Selector::Builder builder;
  builder.SetTagName(element.tag_name());
  if (!element.id().empty())
    builder.SetId(element.id());
  for (auto class_name : element.class_list())
    builder.AddClass(class_name);
  auto style = std::make_unique<css::Style>();
  style_sheet.Merge(style.get(), builder.Build());
  return std::move(style);
}

css::Selector ParseSelector(base::StringPiece16 text) {
  return css::Selector::Parser().Parse(text);
}

}  // namespace

class CompiledStyleSheetSetTest : public ::testing::Test {
 protected:
  CompiledStyleSheetSetTest() = default;
  ~CompiledStyleSheetSetTest() override = default;

  std::vector<css::Selector> Match(const CompiledStyleSheetSet& style_sheet,
                                   const css::Selector& selector) const;

 private:
  DISALLOW_COPY_AND_ASSIGN(CompiledStyleSheetSetTest);
};

std::vector<css::Selector> CompiledStyleSheetSetTest::Match(
    const CompiledStyleSheetSet& style_sheet,
    const css::Selector& selector) const {
  const_cast<CompiledStyleSheetSet&>(style_sheet).CompileStyleSheetsIfNeeded();
  const auto& matched = style_sheet.Match(selector);
  std::vector<css::Selector> result;
  for (const auto& rule : matched)
    result.emplace_back(rule->first);
  return std::move(result);
}

TEST_F(CompiledStyleSheetSetTest, Basic) {
  const auto style_sheet = new css::StyleSheet();
  style_sheet->AppendRule(
      ParseSelector(L"tag"),
      css::StyleBuilder().SetColor(1, 0, 0).SetHeight(20).Build());
  style_sheet->AppendRule(ParseSelector(L".c1"),
                          css::StyleBuilder().SetColor(0, 1, 0).Build());
  style_sheet->AppendRule(ParseSelector(L".c1.c2"),
                          css::StyleBuilder().SetColor(1, 1, 0).Build());
  style_sheet->AppendRule(ParseSelector(L"#id"),
                          css::StyleBuilder().SetColor(0, 0, 1).Build());
  CompiledStyleSheetSet compiled({style_sheet});

  const auto document = NodeTreeBuilder()
                            .Begin(L"tag")
                            .Begin(L"tag", L"id")
                            .End(L"tag")
                            .Begin(L"tag", L"c1")
                            .ClassList({L"c1"})
                            .End(L"tag")
                            .Begin(L"tag", L"c1c2")
                            .ClassList({L"c1", L"c2"})
                            .End(L"tag")
                            .End(L"tag")
                            .Begin(L"tag", L"c1c3")
                            .ClassList({L"c1", L"c3"})
                            .End(L"tag")
                            .Build();

  const auto tag = document->first_child()->as<Element>();
  const auto tag_c1 = document->GetElementById(L"c1");
  const auto tag_c1c2 = document->GetElementById(L"c1c2");
  const auto tag_c1c3 = document->GetElementById(L"c1c3");
  const auto tag_id = document->GetElementById(L"id");

  const auto& tag_style = ComputeStyle(compiled, *tag);
  EXPECT_EQ(css::Color(css::ColorValue(1, 0, 0)), tag_style->color());
  EXPECT_EQ(css::Height(css::Length(20)), tag_style->height());

  const auto& tag_c1_style = ComputeStyle(compiled, *tag_c1);
  EXPECT_EQ(css::Color(css::ColorValue(0, 1, 0)), tag_c1_style->color());
  EXPECT_EQ(css::Height(css::Length(20)), tag_c1_style->height());

  const auto& tag_c1c2_style = ComputeStyle(compiled, *tag_c1c2);
  EXPECT_EQ(css::Color(css::ColorValue(1, 1, 0)), tag_c1c2_style->color());
  EXPECT_EQ(css::Height(css::Length(20)), tag_c1c2_style->height());

  const auto& tag_c1c3_style = ComputeStyle(compiled, *tag_c1c3);
  EXPECT_EQ(css::Color(css::ColorValue(0, 1, 0)), tag_c1c3_style->color());
  EXPECT_EQ(css::Height(css::Length(20)), tag_c1c3_style->height());

  const auto& tag_id_style = ComputeStyle(compiled, *tag_id);
  EXPECT_EQ(css::Color(css::ColorValue(0, 0, 1)), tag_id_style->color());
  EXPECT_EQ(css::Height(css::Length(20)), tag_id_style->height());
}

TEST_F(CompiledStyleSheetSetTest, Match) {
  const auto style_sheet = new css::StyleSheet();
  style_sheet->AppendRule(
      ParseSelector(L"tag"),
      css::StyleBuilder().SetColor(1, 0, 0).SetHeight(20).Build());
  style_sheet->AppendRule(ParseSelector(L".c1"),
                          css::StyleBuilder().SetColor(0, 0, 1).Build());
  style_sheet->AppendRule(ParseSelector(L".c2"),
                          css::StyleBuilder().SetColor(0, 1, 0).Build());
  style_sheet->AppendRule(ParseSelector(L".c1.c2"),
                          css::StyleBuilder().SetColor(1, 1, 0).Build());
  CompiledStyleSheetSet compiled({style_sheet});

  EXPECT_EQ(std::vector<css::Selector>{ParseSelector(L"tag")},
            Match(compiled, ParseSelector(L"tag")));
  EXPECT_EQ(std::vector<css::Selector>{ParseSelector(L".c1")},
            Match(compiled, ParseSelector(L".c1")));
  EXPECT_EQ(std::vector<css::Selector>{ParseSelector(L".c2")},
            Match(compiled, ParseSelector(L".c2")));
  EXPECT_EQ((std::vector<css::Selector>{ParseSelector(L".c1"),
                                        ParseSelector(L"tag")}),
            Match(compiled, ParseSelector(L"tag.c1")));
  EXPECT_EQ((std::vector<css::Selector>{
                ParseSelector(L".c1.c2"), ParseSelector(L".c1"),
                ParseSelector(L".c2"), ParseSelector(L"tag")}),
            Match(compiled, ParseSelector(L"tag.c1.c2")));
  EXPECT_EQ((std::vector<css::Selector>{ParseSelector(L".c1.c2"),
                                        ParseSelector(L".c1"),
                                        ParseSelector(L".c2")}),
            Match(compiled, ParseSelector(L".c1.c2")));
  EXPECT_EQ((std::vector<css::Selector>{ParseSelector(L".c1")}),
            Match(compiled, ParseSelector(L".c1.c3")));
  EXPECT_EQ(std::vector<css::Selector>{ParseSelector(L".c1")},
            Match(compiled, ParseSelector(L".c1:hover")));
}

TEST_F(CompiledStyleSheetSetTest, Hover) {
  const auto style_sheet = new css::StyleSheet();
  style_sheet->AppendRule(
      ParseSelector(L"tag"),
      css::StyleBuilder().SetColor(1, 0, 0).SetHeight(20).Build());
  style_sheet->AppendRule(ParseSelector(L".c1"),
                          css::StyleBuilder().SetColor(0, 0, 1).Build());
  style_sheet->AppendRule(ParseSelector(L".c2"),
                          css::StyleBuilder().SetColor(0, 1, 0).Build());
  CompiledStyleSheetSet compiled({style_sheet});

  const auto document = NodeTreeBuilder()
                            .Begin(L"tag", L"c1")
                            .ClassList({L"c1", L"hover"})
                            .End(L"tag")
                            .Begin(L"tag", L"c2")
                            .ClassList({L"c2"})
                            .End(L"tag")
                            .Build();

  const auto tag_c1 = document->GetElementById(L"c1");
  const auto tag_c2 = document->GetElementById(L"c2");

  const auto& tag_c1_style = ComputeStyle(compiled, *tag_c1);
  const auto& tag_c2_style = ComputeStyle(compiled, *tag_c2);

  EXPECT_EQ(css::Color(css::ColorValue(0, 0, 1)), tag_c1_style->color());
  EXPECT_EQ(css::Height(css::Length(20)), tag_c1_style->height());

  EXPECT_EQ(css::Color(css::ColorValue(0, 1, 0)), tag_c2_style->color());
  EXPECT_EQ(css::Height(css::Length(20)), tag_c2_style->height());
}

}  // namespace visuals
