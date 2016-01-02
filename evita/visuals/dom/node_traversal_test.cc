// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/node_traversal.h"

#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_tree_builder.h"
#include "evita/visuals/dom/text_node.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(NodeTraversalTest, FirstChildOf) {
  const auto& document = NodeTreeBuilder()
                             .Begin(L"block")
                             .AddText(L"foo")
                             .AddText(L"bar")
                             .End(L"block")
                             .Build();
  const auto block = document->first_child()->as<ContainerNode>();

  EXPECT_EQ(block->first_child(), NodeTraversal::FirstChildOf(*block));
  EXPECT_EQ(nullptr,
            NodeTraversal::FirstChildOf(*NodeTraversal::FirstChildOf(*block)));
}

TEST(NodeTraversalTest, FirstChildOfNoChild) {
  const auto& document =
      NodeTreeBuilder().Begin(L"block").End(L"block").Build();
  const auto block = document->first_child();
  EXPECT_EQ(nullptr, NodeTraversal::FirstChildOf(*block));
}

TEST(NodeTraversalTest, LastChildOf) {
  const auto& document = NodeTreeBuilder()
                             .Begin(L"block")
                             .AddText(L"foo")
                             .AddText(L"bar")
                             .End(L"block")
                             .Build();
  const auto block = document->first_child()->as<ContainerNode>();

  EXPECT_EQ(block->last_child(), NodeTraversal::LastChildOf(*block));
  EXPECT_EQ(nullptr,
            NodeTraversal::LastChildOf(*NodeTraversal::LastChildOf(*block)));
}

TEST(NodeTraversalTest, LastChildOfNoChild) {
  const auto& document =
      NodeTreeBuilder().Begin(L"block").End(L"block").Build();
  const auto block = document->first_child();
  EXPECT_EQ(nullptr, NodeTraversal::LastChildOf(*block));
}

TEST(NodeTraversalTest, NextOf) {
  const auto& document = NodeTreeBuilder()
                             .Begin(L"block")
                             .AddText(L"foo")
                             .AddText(L"bar")
                             .End(L"block")
                             .Build();
  const auto block = document->first_child();

  EXPECT_EQ(NodeTraversal::FirstChildOf(*block), NodeTraversal::NextOf(*block));
  EXPECT_EQ(NodeTraversal::LastChildOf(*block),
            NodeTraversal::NextOf(*NodeTraversal::FirstChildOf(*block)));
  EXPECT_EQ(nullptr,
            NodeTraversal::NextOf(*NodeTraversal::LastChildOf(*block)));
}

TEST(NodeTraversalTest, NextSiblingOf) {
  const auto& document = NodeTreeBuilder()
                             .Begin(L"block")
                             .AddText(L"foo")
                             .AddText(L"bar")
                             .End(L"block")
                             .Build();
  const auto block = document->first_child();

  EXPECT_EQ(nullptr, NodeTraversal::NextSiblingOf(*block));
  EXPECT_EQ(NodeTraversal::LastChildOf(*block),
            NodeTraversal::NextSiblingOf(*NodeTraversal::FirstChildOf(*block)));
}

}  // namespace visuals
