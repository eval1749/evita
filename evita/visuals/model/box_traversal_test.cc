// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/model/box_traversal.h"

#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/box_builder.h"
#include "evita/visuals/model/text_box.h"
#include "gtest/gtest.h"

namespace visuals {

namespace {
std::unique_ptr<Box> NewText(const base::string16& text) {
  return std::move(BoxBuilder::New<TextBox>(text).Finish());
}
}  // namespace

TEST(BoxTraversalTest, FirstChildOf) {
  const auto& block = BoxBuilder::New<BlockBox>()
                          .Append(NewText(L"foo"))
                          .Append(NewText(L"bar"))
                          .Finish();
  EXPECT_EQ(block->as<ContainerBox>()->child_boxes().front(),
            BoxTraversal::FirstChildOf(*block));
  EXPECT_EQ(nullptr,
            BoxTraversal::FirstChildOf(*BoxTraversal::FirstChildOf(*block)));
}

TEST(BoxTraversalTest, FirstChildOfNoChild) {
  const auto& block = BoxBuilder::New<BlockBox>().Finish();
  EXPECT_EQ(nullptr, BoxTraversal::FirstChildOf(*block));
}

TEST(BoxTraversalTest, LastChildOf) {
  const auto& block = BoxBuilder::New<BlockBox>()
                          .Append(NewText(L"foo"))
                          .Append(NewText(L"bar"))
                          .Finish();
  EXPECT_EQ(block->as<ContainerBox>()->child_boxes().back(),
            BoxTraversal::LastChildOf(*block));
  EXPECT_EQ(nullptr,
            BoxTraversal::LastChildOf(*BoxTraversal::LastChildOf(*block)));
}

TEST(BoxTraversalTest, LastChildOfNoChild) {
  const auto& block = BoxBuilder::New<BlockBox>().Finish();
  EXPECT_EQ(nullptr, BoxTraversal::LastChildOf(*block));
}

TEST(BoxTraversalTest, NextOf) {
  const auto& block = BoxBuilder::New<BlockBox>()
                          .Append(NewText(L"foo"))
                          .Append(NewText(L"bar"))
                          .Finish();
  EXPECT_EQ(BoxTraversal::FirstChildOf(*block), BoxTraversal::NextOf(*block));
  EXPECT_EQ(BoxTraversal::LastChildOf(*block),
            BoxTraversal::NextOf(*BoxTraversal::FirstChildOf(*block)));
  EXPECT_EQ(nullptr, BoxTraversal::NextOf(*BoxTraversal::LastChildOf(*block)));
}

TEST(BoxTraversalTest, NextSiblingOf) {
  const auto& block = BoxBuilder::New<BlockBox>()
                          .Append(NewText(L"foo"))
                          .Append(NewText(L"bar"))
                          .Finish();
  EXPECT_EQ(nullptr, BoxTraversal::NextSiblingOf(*block));
  EXPECT_EQ(BoxTraversal::LastChildOf(*block),
            BoxTraversal::NextSiblingOf(*BoxTraversal::FirstChildOf(*block)));
}

}  // namespace visuals
