// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/layout/box_editor.h"
#include "evita/visuals/layout/flow_box.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/layout/simple_box_tree_builder.h"
#include "evita/visuals/layout/text_box.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(ContainerBoxTest, AppendChild) {
  const auto& root = SimpleBoxTreeBuilder()
                         .Begin<FlowBox>()
                         .Add<TextBox>(L"foo")
                         .Add<TextBox>(L"bar")
                         .End<FlowBox>()
                         .Build();
  const auto main = root->first_child()->as<ContainerBox>();
  const auto text_box1 = main->first_child();
  const auto text_box2 = main->last_child();

  EXPECT_EQ(main, text_box1->parent());
  EXPECT_EQ(main, text_box2->parent());

  BoxEditor().RemoveDescendants(root.get());
}

TEST(ContainerBoxTest, RemoveChild) {
  const auto& root = SimpleBoxTreeBuilder()
                         .Begin<FlowBox>()
                         .Add<TextBox>(L"foo")
                         .Add<TextBox>(L"bar")
                         .End<FlowBox>()
                         .Build();
  const auto main = root->first_child()->as<ContainerBox>();
  const auto text_box1 = main->first_child();
  const auto text_box2 = main->last_child();

  BoxEditor().RemoveChild(main, text_box2);

  EXPECT_EQ(main, text_box1->parent());
  EXPECT_EQ(nullptr, text_box2->parent());

  BoxEditor().RemoveDescendants(root.get());
}

}  // namespace visuals
