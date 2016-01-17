// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/layout/box_traversal.h"

#include "evita/visuals/layout/box_editor.h"
#include "evita/visuals/layout/flow_box.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/layout/simple_box_tree.h"
#include "evita/visuals/layout/text_box.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(BoxTraversalTest, FirstChildOf) {
  SimpleBoxTree box_tree;
  box_tree.Begin<FlowBox>()
      .Add<TextBox>(L"foo")
      .Add<TextBox>(L"bar")
      .End<FlowBox>();
  const auto root = box_tree.root_box();
  const auto block = root->first_child()->as<ContainerBox>();

  EXPECT_EQ(block->first_child(), BoxTraversal::FirstChildOf(*block));
  EXPECT_EQ(nullptr,
            BoxTraversal::FirstChildOf(*BoxTraversal::FirstChildOf(*block)));
}

TEST(BoxTraversalTest, FirstChildOfNoChild) {
  SimpleBoxTree box_tree;
  box_tree.Add<FlowBox>();
  const auto root = box_tree.root_box();
  const auto block = root->first_child();
  EXPECT_EQ(nullptr, BoxTraversal::FirstChildOf(*block));
}

TEST(BoxTraversalTest, LastChildOf) {
  SimpleBoxTree box_tree;
  box_tree.Begin<FlowBox>()
      .Add<TextBox>(L"foo")
      .Add<TextBox>(L"bar")
      .End<FlowBox>();
  const auto root = box_tree.root_box();
  const auto block = root->first_child()->as<ContainerBox>();

  EXPECT_EQ(block->last_child(), BoxTraversal::LastChildOf(*block));
  EXPECT_EQ(nullptr,
            BoxTraversal::LastChildOf(*BoxTraversal::LastChildOf(*block)));
}

TEST(BoxTraversalTest, LastChildOfNoChild) {
  SimpleBoxTree box_tree;
  box_tree.Add<FlowBox>();
  const auto block = box_tree.root_box()->first_child();
  EXPECT_EQ(nullptr, BoxTraversal::LastChildOf(*block));
}

TEST(BoxTraversalTest, NextOf) {
  SimpleBoxTree box_tree;
  box_tree.Begin<FlowBox>()
      .Add<TextBox>(L"foo")
      .Add<TextBox>(L"bar")
      .End<FlowBox>();
  const auto root = box_tree.root_box();
  const auto block = root->first_child();

  EXPECT_EQ(BoxTraversal::FirstChildOf(*block), BoxTraversal::NextOf(*block));
  EXPECT_EQ(BoxTraversal::LastChildOf(*block),
            BoxTraversal::NextOf(*BoxTraversal::FirstChildOf(*block)));
  EXPECT_EQ(nullptr, BoxTraversal::NextOf(*BoxTraversal::LastChildOf(*block)));
}

TEST(BoxTraversalTest, NextSiblingOf) {
  SimpleBoxTree box_tree;
  box_tree.Begin<FlowBox>()
      .Add<TextBox>(L"foo")
      .Add<TextBox>(L"bar")
      .End<FlowBox>();
  const auto root = box_tree.root_box();
  const auto block = root->first_child();

  EXPECT_EQ(nullptr, BoxTraversal::NextSiblingOf(*block));
  EXPECT_EQ(BoxTraversal::LastChildOf(*block),
            BoxTraversal::NextSiblingOf(*BoxTraversal::FirstChildOf(*block)));
}

}  // namespace visuals
