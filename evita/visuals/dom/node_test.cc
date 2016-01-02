// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/dom/node_tree_builder.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/text_node.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(NodeTest, InitialValues) {
  const auto& document = NodeTreeBuilder().AddText(L"foo").Build();
  const auto& node = document->first_child();
  EXPECT_EQ(css::Background(), node->background());
  EXPECT_EQ(css::Border(), node->border());
  EXPECT_FALSE(node->is_display_none());
  EXPECT_EQ(css::Margin(), node->margin());
  EXPECT_EQ(css::Padding(), node->padding());
  EXPECT_EQ(css::Position::Static(), node->position());
}

TEST(NodeTest, IsDescendantOf) {
  const auto& document = NodeTreeBuilder()
                             .Begin(L"block")
                             .AddText(L"foo")
                             .AddText(L"bar")
                             .End(L"block")
                             .Build();
  const auto main = document->first_child()->as<ContainerNode>();
  const auto text_node1 = main->first_child();
  const auto text_node2 = main->last_child();

  EXPECT_FALSE(document->IsDescendantOf(*document));
  EXPECT_TRUE(main->IsDescendantOf(*document));
  EXPECT_TRUE(text_node1->IsDescendantOf(*main));
  EXPECT_TRUE(text_node2->IsDescendantOf(*main));
  EXPECT_FALSE(text_node2->IsDescendantOf(*text_node1));
}

}  // namespace visuals
