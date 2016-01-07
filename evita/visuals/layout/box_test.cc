// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/layout/block_flow_box.h"
#include "evita/visuals/layout/box_editor.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/layout/simple_box_tree_builder.h"
#include "evita/visuals/layout/text_box.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(BoxTest, InitialValues) {
  const auto& root = SimpleBoxTreeBuilder().Add<TextBox>(L"foo").Build();
  const auto& box = root->first_child();

  EXPECT_EQ(css::Background(), box->background());
  EXPECT_EQ(css::Border(), box->border());
  EXPECT_EQ(css::Display(), box->display());
  EXPECT_EQ(css::Margin(), box->margin());
  EXPECT_EQ(css::Padding(), box->padding());
  EXPECT_EQ(css::Position::Static(), box->position());

  BoxEditor().RemoveDescendants(root.get());
}

TEST(BoxTest, IsDescendantOf) {
  const auto& root = SimpleBoxTreeBuilder()
                         .Begin<BlockFlowBox>()
                         .Add<TextBox>(L"foo")
                         .Add<TextBox>(L"bar")
                         .End<BlockFlowBox>()
                         .Build();
  const auto main = root->first_child()->as<ContainerBox>();
  const auto text_box1 = main->first_child();
  const auto text_box2 = main->last_child();

  EXPECT_FALSE(root->IsDescendantOf(*root));
  EXPECT_TRUE(main->IsDescendantOf(*root));
  EXPECT_TRUE(text_box1->IsDescendantOf(*main));
  EXPECT_TRUE(text_box2->IsDescendantOf(*main));
  EXPECT_FALSE(text_box2->IsDescendantOf(*text_box1));

  BoxEditor().RemoveDescendants(root.get());
}

}  // namespace visuals
