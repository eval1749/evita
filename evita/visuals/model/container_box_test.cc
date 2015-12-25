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
  auto text_box1 = std::make_unique<TextBox>(L"foo");
  auto text_box2 = std::make_unique<TextBox>(L"bar");
  root_box->AppendChild(text_box1.get());
  root_box->AppendChild(text_box2.get());

  EXPECT_EQ(root_box.get(), text_box1->parent());
  EXPECT_EQ(root_box.get(), text_box2->parent());
}

TEST(ContainerBoxTest, RemoveChild) {
  auto root_box = std::make_unique<BlockBox>();
  auto text_box1 = std::make_unique<TextBox>(L"foo");
  auto text_box2 = std::make_unique<TextBox>(L"bar");
  root_box->AppendChild(text_box1.get());
  root_box->AppendChild(text_box2.get());

  root_box->RemoveChild(text_box2.get());

  EXPECT_EQ(root_box.get(), text_box1->parent());
  EXPECT_EQ(nullptr, text_box2->parent());
}
}  // namespace visuals
