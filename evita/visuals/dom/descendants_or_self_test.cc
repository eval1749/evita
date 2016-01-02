// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "evita/visuals/dom/descendants_or_self.h"

#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_tree_builder.h"
#include "evita/visuals/dom/text_node.h"
#include "evita/visuals/dom/node_traversal.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(NodeDescendantsOrSelfTest, Basic) {
  const auto& document = NodeTreeBuilder()
                             .Begin(L"block")
                             .AddText(L"a")
                             .Begin(L"block")
                             .AddText(L"b")
                             .AddText(L"c")
                             .End(L"block")
                             .AddText(L"d")
                             .End(L"block")
                             .Build();
  const auto block = document->first_child();
  std::vector<Node*> visited;
  for (const auto& runner : Node::DescendantsOrSelf(*block))
    visited.push_back(runner);
  EXPECT_EQ(6, visited.size());
  EXPECT_EQ(block, visited.front());
  EXPECT_EQ(NodeTraversal::LastChildOf(*block), visited.back());
}

TEST(NodeDescendantsOrSelfTest, NoChild) {
  const auto& document =
      NodeTreeBuilder().Begin(L"block").End(L"block").Build();
  const auto block = document->first_child();
  std::vector<Node*> visited;
  for (const auto& runner : Node::DescendantsOrSelf(*block))
    visited.push_back(runner);
  EXPECT_EQ(1, visited.size());
  EXPECT_EQ(block, visited.front());
}
}  // namespace visuals
