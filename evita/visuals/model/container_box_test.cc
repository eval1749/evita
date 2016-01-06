// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/model/block_flow_box.h"
#include "evita/visuals/model/box_editor.h"
#include "evita/visuals/model/root_box.h"
#include "evita/visuals/model/simple_box_tree_builder.h"
#include "evita/visuals/model/text_box.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(ContainerBoxTest, AppendChild) {
  const auto& root = SimpleBoxTreeBuilder()
                         .Begin<BlockFlowBox>()
                         .Add<TextBox>(L"foo")
                         .Add<TextBox>(L"bar")
                         .End<BlockFlowBox>()
                         .Build();
  const auto main = root->first_child()->as<ContainerBox>();
  const auto text_box1 = main->first_child();
  const auto text_box2 = main->last_child();

  EXPECT_EQ(main, text_box1->parent());
  EXPECT_EQ(main, text_box2->parent());
}

TEST(ContainerBoxTest, RemoveChild) {
  const auto& root = SimpleBoxTreeBuilder()
                         .Begin<BlockFlowBox>()
                         .Add<TextBox>(L"foo")
                         .Add<TextBox>(L"bar")
                         .End<BlockFlowBox>()
                         .Build();
  const auto main = root->first_child()->as<ContainerBox>();
  const auto text_box1 = main->first_child();
  const auto text_box2 = main->last_child();

  BoxEditor().RemoveChild(main, text_box2);

  EXPECT_EQ(main, text_box1->parent());
  EXPECT_EQ(nullptr, text_box2->parent());
}
}  // namespace visuals
