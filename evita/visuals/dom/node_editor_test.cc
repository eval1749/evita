// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/node_editor.h"

#include "evita/visuals/dom/node_tree_builder.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/text.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(NodeTest, ReplaceChild) {
  const auto& document = NodeTreeBuilder()
                             .Begin(L"block")
                             .AddText(L"foo")
                             .AddText(L"bar")
                             .AddText(L"baz")
                             .End(L"block")
                             .Build();
  const auto main = document->first_child()->as<ContainerNode>();
  const auto text1 = main->first_child();
  const auto text2 = text1->next_sibling();
  const auto text3 = main->last_child();
  NodeEditor().ReplaceChild(main, text1, text2);

  EXPECT_EQ(main, text1->parent());
  EXPECT_EQ(text3, text1->next_sibling());
  EXPECT_EQ(nullptr, text1->previous_sibling());

  EXPECT_EQ(nullptr, text2->parent());
  EXPECT_EQ(nullptr, text2->next_sibling());
  EXPECT_EQ(nullptr, text2->previous_sibling());

  EXPECT_EQ(main, text3->parent());
  EXPECT_EQ(nullptr, text3->next_sibling());
  EXPECT_EQ(text1, text3->previous_sibling());
}

}  // namespace visuals
