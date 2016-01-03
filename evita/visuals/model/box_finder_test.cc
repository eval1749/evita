// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/layout/layouter.h"
#include "evita/visuals/model/block_flow_box.h"
#include "evita/visuals/model/box_editor.h"
#include "evita/visuals/model/box_finder.h"
#include "evita/visuals/model/box_tree_builder.h"
#include "evita/visuals/model/root_box.h"
#include "evita/visuals/model/text_box.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(BoxFinderTest, Basic) {
  const auto& root = BoxTreeBuilder()
                         .Begin<BlockFlowBox>()
                         .Add<TextBox>(L"foo")
                         .Add<TextBox>(L"bar")
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
}

}  // namespace visuals
