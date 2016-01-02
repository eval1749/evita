// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_editor.h"
#include "evita/visuals/dom/node_tree_builder.h"
#include "evita/visuals/dom/text_node.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(ContainerNodeTest, AppendChild) {
  const auto& document = NodeTreeBuilder()
                             .Begin(L"block")
                             .AddText(L"foo")
                             .AddText(L"bar")
                             .End(L"block")
                             .Build();
  const auto main = document->first_child()->as<ContainerNode>();
  const auto text_node1 = main->first_child();
  const auto text_node2 = main->last_child();

  EXPECT_EQ(main, text_node1->parent());
  EXPECT_EQ(main, text_node2->parent());
}

TEST(ContainerNodeTest, RemoveChild) {
  const auto& document = NodeTreeBuilder()
                             .Begin(L"block")
                             .AddText(L"foo")
                             .AddText(L"bar")
                             .End(L"block")
                             .Build();
  const auto main = document->first_child()->as<ContainerNode>();
  const auto text_node1 = main->first_child();
  const auto text_node2 = main->last_child();

  const auto& removed_child = NodeEditor().RemoveChild(main, text_node2);

  EXPECT_EQ(main, text_node1->parent());
  EXPECT_EQ(nullptr, text_node2->parent());
  EXPECT_EQ(text_node2, removed_child.get());
}
}  // namespace visuals
