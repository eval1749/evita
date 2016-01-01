// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/box_traversal.h"

#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/box_tree_builder.h"
#include "evita/visuals/model/root_box.h"
#include "evita/visuals/model/text_box.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(BoxTraversalTest, FirstChildOf) {
  const auto& root = BoxTreeBuilder()
                         .Begin<BlockBox>()
                         .Add<TextBox>(L"foo")
                         .Add<TextBox>(L"bar")
                         .End<BlockBox>()
                         .Build();
  const auto block = root->first_child();

  EXPECT_EQ(block->as<ContainerBox>()->child_boxes().front(),
            BoxTraversal::FirstChildOf(*block));
  EXPECT_EQ(nullptr,
            BoxTraversal::FirstChildOf(*BoxTraversal::FirstChildOf(*block)));
}

TEST(BoxTraversalTest, FirstChildOfNoChild) {
  const auto& root = BoxTreeBuilder().Add<BlockBox>().Build();
  const auto block = root->first_child();
  EXPECT_EQ(nullptr, BoxTraversal::FirstChildOf(*block));
}

TEST(BoxTraversalTest, LastChildOf) {
  const auto& root = BoxTreeBuilder()
                         .Begin<BlockBox>()
                         .Add<TextBox>(L"foo")
                         .Add<TextBox>(L"bar")
                         .End<BlockBox>()
                         .Build();
  const auto block = root->first_child();

  EXPECT_EQ(block->as<ContainerBox>()->child_boxes().back(),
            BoxTraversal::LastChildOf(*block));
  EXPECT_EQ(nullptr,
            BoxTraversal::LastChildOf(*BoxTraversal::LastChildOf(*block)));
}

TEST(BoxTraversalTest, LastChildOfNoChild) {
  const auto& root = BoxTreeBuilder().Add<BlockBox>().Build();
  const auto block = root->first_child();
  EXPECT_EQ(nullptr, BoxTraversal::LastChildOf(*block));
}

TEST(BoxTraversalTest, NextOf) {
  const auto& root = BoxTreeBuilder()
                         .Begin<BlockBox>()
                         .Add<TextBox>(L"foo")
                         .Add<TextBox>(L"bar")
                         .End<BlockBox>()
                         .Build();
  const auto block = root->first_child();

  EXPECT_EQ(BoxTraversal::FirstChildOf(*block), BoxTraversal::NextOf(*block));
  EXPECT_EQ(BoxTraversal::LastChildOf(*block),
            BoxTraversal::NextOf(*BoxTraversal::FirstChildOf(*block)));
  EXPECT_EQ(nullptr, BoxTraversal::NextOf(*BoxTraversal::LastChildOf(*block)));
}

TEST(BoxTraversalTest, NextSiblingOf) {
  const auto& root = BoxTreeBuilder()
                         .Begin<BlockBox>()
                         .Add<TextBox>(L"foo")
                         .Add<TextBox>(L"bar")
                         .End<BlockBox>()
                         .Build();
  const auto block = root->first_child();

  EXPECT_EQ(nullptr, BoxTraversal::NextSiblingOf(*block));
  EXPECT_EQ(BoxTraversal::LastChildOf(*block),
            BoxTraversal::NextSiblingOf(*BoxTraversal::FirstChildOf(*block)));
}

}  // namespace visuals
