// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_tree_builder.h"
#include "evita/visuals/dom/text.h"
#include "evita/visuals/layout/block_flow_box.h"
#include "evita/visuals/layout/box_editor.h"
#include "evita/visuals/layout/box_finder.h"
#include "evita/visuals/layout/layouter.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/layout/simple_box_tree_builder.h"
#include "evita/visuals/layout/text_box.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(BoxFinderTest, Basic) {
  const auto document = NodeTreeBuilder()
                            .Begin(L"block")
                            .AddText(L"foo")
                            .AddText(L"bar")
                            .End(L"block")
                            .Build();
  const auto block = document->first_child()->as<Element>();
  const auto text1 = block->first_child()->as<Text>();
  const auto text2 = block->last_child()->as<Text>();
  const auto& root = SimpleBoxTreeBuilder()
                         .Begin<BlockFlowBox>(block)
                         .Add<TextBox>(text1->data(), text1)
                         .Add<TextBox>(text2->data(), text2)
                         .End<BlockFlowBox>()
                         .Build();
  const auto main = root->first_child()->as<ContainerBox>();
  const auto text_box1 = main->first_child();
  const auto text_box2 = main->last_child();
  BoxEditor().SetViewportSize(root.get(), FloatSize(640, 480));
  Layouter().Layout(root.get());

  BoxFinder finder(*root);

  EXPECT_EQ(BoxFinder::Result(), finder.FindByPoint(FloatPoint(-1, -1)));
  EXPECT_EQ(BoxFinder::Result(), finder.FindByPoint(FloatPoint(9999, 9999)));
  EXPECT_EQ(BoxFinder::Result(text_box1, FloatPoint(1, 2)),
            finder.FindByPoint(text_box1->bounds().origin() + FloatSize(1, 2)));
  EXPECT_EQ(BoxFinder::Result(text_box2, FloatPoint(3, 4)),
            finder.FindByPoint(text_box2->bounds().origin() + FloatSize(3, 4)));

  BoxEditor().RemoveDescendants(root.get());
}

}  // namespace visuals
