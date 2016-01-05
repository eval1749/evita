// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_editor.h"
#include "evita/visuals/dom/node_tree_builder.h"
#include "evita/visuals/dom/text.h"
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
  const auto text1 = main->first_child();
  const auto text2 = main->last_child();

  EXPECT_EQ(main, text1->parent());
  EXPECT_EQ(text2, text1->next_sibling());
  EXPECT_EQ(nullptr, text1->previous_sibling());
  EXPECT_EQ(main, text2->parent());
  EXPECT_EQ(nullptr, text2->next_sibling());
  EXPECT_EQ(text1, text2->previous_sibling());
}

TEST(ContainerNodeTest, InsertBerore) {
  const auto& document = NodeTreeBuilder()
                             .Begin(L"block")
                             .AddText(L"foo")
                             .AddText(L"bar")
                             .End(L"block")
                             .Build();
  const auto main = document->first_child()->as<ContainerNode>();
  const auto text1 = main->first_child();
  const auto text2 = main->last_child();
  const auto text3 = new Text(document, L"baz");
  NodeEditor().InsertBefore(main, text3, text2);

  EXPECT_EQ(main, text2->parent());
  EXPECT_EQ(nullptr, text2->next_sibling());
  EXPECT_EQ(text3, text2->previous_sibling());

  EXPECT_EQ(main, text3->parent());
  EXPECT_EQ(text2, text3->next_sibling());
  EXPECT_EQ(text1, text3->previous_sibling());
}

TEST(ContainerNodeTest, RemoveChild) {
  const auto& document = NodeTreeBuilder()
                             .Begin(L"block")
                             .AddText(L"foo")
                             .AddText(L"bar")
                             .End(L"block")
                             .Build();
  const auto main = document->first_child()->as<ContainerNode>();
  const auto text1 = main->first_child();
  const auto text2 = main->last_child();

  NodeEditor().RemoveChild(main, text2);

  EXPECT_EQ(main, text1->parent());
  EXPECT_EQ(nullptr, text2->parent());
  EXPECT_EQ(nullptr, text2->next_sibling());
  EXPECT_EQ(nullptr, text2->previous_sibling());
}
}  // namespace visuals
