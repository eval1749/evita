// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/layout/border.h"
#include "evita/visuals/layout/box_editor.h"
#include "evita/visuals/layout/flow_box.h"
#include "evita/visuals/layout/margin.h"
#include "evita/visuals/layout/padding.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/layout/simple_box_tree.h"
#include "evita/visuals/layout/text_box.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(BoxTest, InitialValues) {
  SimpleBoxTree box_tree;
  box_tree.Add<TextBox>(L"foo").Finish();
  const auto root = box_tree.root_box();
  const auto& box = root->first_child();
  const auto& margin = box->ComputeMargin();
  const auto& padding = box->ComputePadding();

  EXPECT_EQ(FloatColor(), box->background_color());
  EXPECT_EQ(Border(), box->ComputeBorder());
  EXPECT_EQ(css::Display(), box->display());
  EXPECT_EQ(0.0f, margin.bottom());
  EXPECT_EQ(0.0f, margin.left());
  EXPECT_EQ(0.0f, margin.right());
  EXPECT_EQ(0.0f, margin.top());
  EXPECT_EQ(0.0f, padding.bottom());
  EXPECT_EQ(0.0f, padding.left());
  EXPECT_EQ(0.0f, padding.right());
  EXPECT_EQ(0.0f, padding.top());
  EXPECT_EQ(css::Position::Static(), box->position());
}

TEST(BoxTest, IsDescendantOf) {
  SimpleBoxTree box_tree;
  box_tree.Begin<FlowBox>()
      .Add<TextBox>(L"foo")
      .Add<TextBox>(L"bar")
      .End<FlowBox>()
      .Finish();
  const auto root = box_tree.root_box();
  const auto main = root->first_child()->as<ContainerBox>();
  const auto text_box1 = main->first_child();
  const auto text_box2 = main->last_child();

  EXPECT_FALSE(root->IsDescendantOf(*root));
  EXPECT_TRUE(main->IsDescendantOf(*root));
  EXPECT_TRUE(text_box1->IsDescendantOf(*main));
  EXPECT_TRUE(text_box2->IsDescendantOf(*main));
  EXPECT_FALSE(text_box2->IsDescendantOf(*text_box1));
}

}  // namespace visuals
