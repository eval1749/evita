// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_builder.h"
#include "evita/visuals/css/values.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_tree_builder.h"
#include "evita/visuals/dom/text.h"
#include "evita/visuals/layout/box_editor.h"
#include "evita/visuals/layout/box_finder.h"
#include "evita/visuals/layout/flow_box.h"
#include "evita/visuals/layout/layouter.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/layout/simple_box_tree.h"
#include "evita/visuals/layout/text_box.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(BoxFinderTest, Basic) {
  const auto& style = css::StyleBuilder()
                          .SetFontFamily(css::FontFamily(css::String(L"Arial")))
                          .SetFontSize(css::FontSize(css::Length(10)))
                          .Build();
  const auto document = NodeTreeBuilder()
                            .Begin(L"block")
                            .AddText(L"foo")
                            .AddText(L"bar")
                            .End(L"block")
                            .Build();
  const auto block = document->first_child()->as<Element>();
  const auto text1 = block->first_child()->as<Text>();
  const auto text2 = block->last_child()->as<Text>();
  SimpleBoxTree box_tree(*document);
  box_tree.Begin<FlowBox>(block)
      .Add<TextBox>(text1->data(), text1)
      .Add<TextBox>(text2->data(), text2)
      .End<FlowBox>();
  const auto root = box_tree.root_box();
  const auto main = root->first_child()->as<ContainerBox>();
  const auto text_box1 = main->first_child();
  const auto text_box2 = main->last_child();
  BoxEditor().SetStyle(text_box1, *style);
  BoxEditor().SetStyle(text_box2, *style);
  box_tree.Finish();
  Layouter().Layout(box_tree.root_box());

  BoxFinder finder(*box_tree.root_box());

  EXPECT_EQ(HitTestResult(), finder.FindByPoint(FloatPoint(-1, -1)));
  EXPECT_EQ(HitTestResult(), finder.FindByPoint(FloatPoint(9999, 9999)));
  EXPECT_EQ(HitTestResult(text_box1, FloatPoint(1, 2)),
            finder.FindByPoint(text_box1->bounds().origin() + FloatSize(1, 2)));
  EXPECT_EQ(HitTestResult(text_box2, FloatPoint(3, 4)),
            finder.FindByPoint(text_box2->bounds().origin() + FloatSize(3, 4)));
}

}  // namespace visuals
