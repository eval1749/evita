// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/text_box.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(ContainerBoxTest, AppendChild) {
  auto root_box = std::make_unique<BlockBox>();
  const auto& text_box1 = root_box->AppendNew<TextBox>(L"foo");
  const auto& text_box2 = root_box->AppendNew<TextBox>(L"bar");

  EXPECT_EQ(root_box.get(), text_box1->parent());
  EXPECT_EQ(root_box.get(), text_box2->parent());
}

TEST(ContainerBoxTest, RemoveChild) {
  auto root_box = std::make_unique<BlockBox>();
  const auto& text_box1 = root_box->AppendNew<TextBox>(L"foo");
  const auto& text_box2 = root_box->AppendNew<TextBox>(L"bar");
  const auto& removed_child = root_box->RemoveChild(text_box2);

  EXPECT_EQ(root_box.get(), text_box1->parent());
  EXPECT_EQ(nullptr, text_box2->parent());
  EXPECT_EQ(text_box2, removed_child.get());
}
}  // namespace visuals
