// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_tree_builder.h"
#include "evita/visuals/dom/text.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace visuals {

TEST(NodeTest, Contains) {
  const auto& document = NodeTreeBuilder()
                             .Begin(L"block")
                             .AddText(L"foo")
                             .AddText(L"bar")
                             .End(L"block")
                             .Build();
  const auto main = document->first_child()->as<ContainerNode>();
  const auto text1 = main->first_child();
  const auto text2 = main->last_child();

  EXPECT_TRUE(document->Contains(*document));
  EXPECT_TRUE(document->Contains(*main));
  EXPECT_TRUE(main->Contains(*main));
  EXPECT_TRUE(main->Contains(*text1));
  EXPECT_TRUE(main->Contains(*text2));
  EXPECT_FALSE(text1->Contains(*text2));
}

TEST(NodeTest, IsDescendantOf) {
  const auto& document = NodeTreeBuilder()
                             .Begin(L"block")
                             .AddText(L"foo")
                             .AddText(L"bar")
                             .End(L"block")
                             .Build();
  const auto main = document->first_child()->as<ContainerNode>();
  const auto text1 = main->first_child();
  const auto text2 = main->last_child();

  EXPECT_FALSE(document->IsDescendantOf(*document));
  EXPECT_TRUE(main->IsDescendantOf(*document));
  EXPECT_TRUE(text1->IsDescendantOf(*main));
  EXPECT_TRUE(text2->IsDescendantOf(*main));
  EXPECT_FALSE(text2->IsDescendantOf(*text1));
}

}  // namespace visuals
