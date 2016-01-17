// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "evita/visuals/layout/descendants_or_self.h"

#include "evita/visuals/layout/box_editor.h"
#include "evita/visuals/layout/flow_box.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/layout/simple_box_tree.h"
#include "evita/visuals/layout/text_box.h"
#include "evita/visuals/layout/box_traversal.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(BoxDescendantsOrSelfTest, Basic) {
  SimpleBoxTree box_tree;
  box_tree.Begin<FlowBox>()
      .Add<TextBox>(L"a")
      .Begin<FlowBox>()
      .Add<TextBox>(L"b")
      .Add<TextBox>(L"c")
      .End<FlowBox>()
      .Add<TextBox>(L"d")
      .End<FlowBox>()
      .Finish();
  const auto root = box_tree.root_box();
  const auto block = root->first_child();
  std::vector<Box*> visited;
  for (const auto& runner : Box::DescendantsOrSelf(*block))
    visited.push_back(runner);
  EXPECT_EQ(6, visited.size());
  EXPECT_EQ(block, visited.front());
  EXPECT_EQ(BoxTraversal::LastChildOf(*block), visited.back());
}

TEST(BoxDescendantsOrSelfTest, NoChild) {
  SimpleBoxTree box_tree;
  box_tree.Add<FlowBox>().Finish();
  const auto root = box_tree.root_box();
  const auto block = root->first_child();
  std::vector<Box*> visited;
  for (const auto& runner : Box::DescendantsOrSelf(*block))
    visited.push_back(runner);
  EXPECT_EQ(1, visited.size());
  EXPECT_EQ(block, visited.front());
}
}  // namespace visuals
