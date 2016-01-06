// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/box_traversal.h"

#include "evita/visuals/model/block_flow_box.h"
#include "evita/visuals/model/box_editor.h"
#include "evita/visuals/model/root_box.h"
#include "evita/visuals/model/simple_box_tree_builder.h"
#include "evita/visuals/model/text_box.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(BoxTraversalTest, FirstChildOf) {
  const auto& root = SimpleBoxTreeBuilder()
                         .Begin<BlockFlowBox>()
                         .Add<TextBox>(L"foo")
                         .Add<TextBox>(L"bar")
                         .End<BlockFlowBox>()
                         .Build();
  const auto block = root->first_child()->as<ContainerBox>();

  EXPECT_EQ(block->first_child(), BoxTraversal::FirstChildOf(*block));
  EXPECT_EQ(nullptr,
            BoxTraversal::FirstChildOf(*BoxTraversal::FirstChildOf(*block)));

  BoxEditor().RemoveDescendants(root.get());
}

TEST(BoxTraversalTest, FirstChildOfNoChild) {
  const auto& root = SimpleBoxTreeBuilder().Add<BlockFlowBox>().Build();
  const auto block = root->first_child();
  EXPECT_EQ(nullptr, BoxTraversal::FirstChildOf(*block));

  BoxEditor().RemoveDescendants(root.get());
}

TEST(BoxTraversalTest, LastChildOf) {
  const auto& root = SimpleBoxTreeBuilder()
                         .Begin<BlockFlowBox>()
                         .Add<TextBox>(L"foo")
                         .Add<TextBox>(L"bar")
                         .End<BlockFlowBox>()
                         .Build();
  const auto block = root->first_child()->as<ContainerBox>();

  EXPECT_EQ(block->last_child(), BoxTraversal::LastChildOf(*block));
  EXPECT_EQ(nullptr,
            BoxTraversal::LastChildOf(*BoxTraversal::LastChildOf(*block)));

  BoxEditor().RemoveDescendants(root.get());
}

TEST(BoxTraversalTest, LastChildOfNoChild) {
  const auto& root = SimpleBoxTreeBuilder().Add<BlockFlowBox>().Build();
  const auto block = root->first_child();
  EXPECT_EQ(nullptr, BoxTraversal::LastChildOf(*block));

  BoxEditor().RemoveDescendants(root.get());
}

TEST(BoxTraversalTest, NextOf) {
  const auto& root = SimpleBoxTreeBuilder()
                         .Begin<BlockFlowBox>()
                         .Add<TextBox>(L"foo")
                         .Add<TextBox>(L"bar")
                         .End<BlockFlowBox>()
                         .Build();
  const auto block = root->first_child();

  EXPECT_EQ(BoxTraversal::FirstChildOf(*block), BoxTraversal::NextOf(*block));
  EXPECT_EQ(BoxTraversal::LastChildOf(*block),
            BoxTraversal::NextOf(*BoxTraversal::FirstChildOf(*block)));
  EXPECT_EQ(nullptr, BoxTraversal::NextOf(*BoxTraversal::LastChildOf(*block)));

  BoxEditor().RemoveDescendants(root.get());
}

TEST(BoxTraversalTest, NextSiblingOf) {
  const auto& root = SimpleBoxTreeBuilder()
                         .Begin<BlockFlowBox>()
                         .Add<TextBox>(L"foo")
                         .Add<TextBox>(L"bar")
                         .End<BlockFlowBox>()
                         .Build();
  const auto block = root->first_child();

  EXPECT_EQ(nullptr, BoxTraversal::NextSiblingOf(*block));
  EXPECT_EQ(BoxTraversal::LastChildOf(*block),
            BoxTraversal::NextSiblingOf(*BoxTraversal::FirstChildOf(*block)));

  BoxEditor().RemoveDescendants(root.get());
}

}  // namespace visuals
