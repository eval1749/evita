// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/box_builder.h"
#include "evita/visuals/model/box_editor.h"
#include "evita/visuals/model/text_box.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(ContainerBoxTest, AppendChild) {
  const auto& root_box = BoxBuilder(new BlockBox())
                             .Append(BoxBuilder(new TextBox(L"foo")).Finish())
                             .Append(BoxBuilder(new TextBox(L"bar")).Finish())
                             .Finish();
  const auto text_box1 = root_box->as<ContainerBox>()->child_boxes()[0];
  const auto text_box2 = root_box->as<ContainerBox>()->child_boxes()[1];

  EXPECT_EQ(root_box.get(), text_box1->parent());
  EXPECT_EQ(root_box.get(), text_box2->parent());
}

TEST(ContainerBoxTest, RemoveChild) {
  const auto& root_box = BoxBuilder(new BlockBox())
                             .Append(BoxBuilder(new TextBox(L"foo")).Finish())
                             .Append(BoxBuilder(new TextBox(L"bar")).Finish())
                             .Finish();
  const auto text_box1 = root_box->as<ContainerBox>()->child_boxes()[0];
  const auto text_box2 = root_box->as<ContainerBox>()->child_boxes()[1];

  const auto& removed_child =
      BoxEditor().RemoveChild(root_box->as<ContainerBox>(), text_box2);

  EXPECT_EQ(root_box.get(), text_box1->parent());
  EXPECT_EQ(nullptr, text_box2->parent());
  EXPECT_EQ(text_box2, removed_child.get());
}
}  // namespace visuals
