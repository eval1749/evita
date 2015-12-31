// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/box_builder.h"
#include "evita/visuals/model/text_box.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(BoxTest, InitialValues) {
  const auto& box = BoxBuilder::New<TextBox>(L"foo").Finish();
  EXPECT_EQ(css::Background(), box->background());
  EXPECT_EQ(css::Border(), box->border());
  EXPECT_FALSE(box->is_display_none());
  EXPECT_EQ(css::Margin(), box->margin());
  EXPECT_EQ(css::Padding(), box->padding());
  EXPECT_EQ(css::Position::Static(), box->position());
}

TEST(BoxTest, IsDescendantOf) {
  const auto& root_box = BoxBuilder::New<BlockBox>()
                             .Append(BoxBuilder::New<TextBox>(L"foo").Finish())
                             .Append(BoxBuilder::New<TextBox>(L"bar").Finish())
                             .Finish();
  const auto text_box1 = root_box->as<ContainerBox>()->child_boxes()[0];
  const auto text_box2 = root_box->as<ContainerBox>()->child_boxes()[1];

  EXPECT_FALSE(root_box->IsDescendantOf(*root_box));
  EXPECT_TRUE(text_box1->IsDescendantOf(*root_box));
  EXPECT_TRUE(text_box2->IsDescendantOf(*root_box));
  EXPECT_FALSE(text_box2->IsDescendantOf(*text_box1));
}

}  // namespace visuals
