// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "evita/visuals/model/descendants_or_self.h"

#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/box_builder.h"
#include "evita/visuals/model/text_box.h"
#include "evita/visuals/model/box_traversal.h"
#include "gtest/gtest.h"

namespace visuals {

namespace {
std::unique_ptr<Box> NewText(const base::string16& text) {
  return std::move(BoxBuilder::New<TextBox>(text).Finish());
}
}  // namespace

TEST(BoxDescendantsOrSelfTest, Basic) {
  const auto& block = BoxBuilder::New<BlockBox>()
                          .Append(NewText(L"a"))
                          .Append(BoxBuilder::New<BlockBox>()
                                      .Append(NewText(L"b"))
                                      .Append(NewText(L"c"))
                                      .Finish())
                          .Append(NewText(L"d"))
                          .Finish();
  std::vector<Box*> visited;
  for (const auto& runner : Box::DescendantsOrSelf(*block))
    visited.push_back(runner);
  EXPECT_EQ(6, visited.size());
  EXPECT_EQ(block.get(), visited.front());
  EXPECT_EQ(BoxTraversal::LastChildOf(*block), visited.back());
}

TEST(BoxDescendantsOrSelfTest, NoChild) {
  const auto& block = BoxBuilder::New<BlockBox>().Finish();
  std::vector<Box*> visited;
  for (const auto& runner : Box::DescendantsOrSelf(*block))
    visited.push_back(runner);
  EXPECT_EQ(1, visited.size());
  EXPECT_EQ(block.get(), visited.front());
}
}  // namespace visuals
