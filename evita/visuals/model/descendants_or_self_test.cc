// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "evita/visuals/model/descendants_or_self.h"

#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/box_tree_builder.h"
#include "evita/visuals/model/root_box.h"
#include "evita/visuals/model/text_box.h"
#include "evita/visuals/model/box_traversal.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(BoxDescendantsOrSelfTest, Basic) {
  const auto& root = BoxTreeBuilder()
                         .Begin<BlockBox>()
                         .Add<TextBox>(L"a")
                         .Begin<BlockBox>()
                         .Add<TextBox>(L"b")
                         .Add<TextBox>(L"c")
                         .End<BlockBox>()
                         .Add<TextBox>(L"d")
                         .End<BlockBox>()
                         .Build();
  const auto block = root->first_child();
  std::vector<Box*> visited;
  for (const auto& runner : Box::DescendantsOrSelf(*block))
    visited.push_back(runner);
  EXPECT_EQ(6, visited.size());
  EXPECT_EQ(block, visited.front());
  EXPECT_EQ(BoxTraversal::LastChildOf(*block), visited.back());
}

TEST(BoxDescendantsOrSelfTest, NoChild) {
  const auto& root = BoxTreeBuilder().Add<BlockBox>().Build();
  const auto block = root->first_child();
  std::vector<Box*> visited;
  for (const auto& runner : Box::DescendantsOrSelf(*block))
    visited.push_back(runner);
  EXPECT_EQ(1, visited.size());
  EXPECT_EQ(block, visited.front());
}
}  // namespace visuals
