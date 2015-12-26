// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/text_box.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(BoxTest, IsDescendantOf) {
  auto root_box = std::make_unique<BlockBox>();
  const auto& text_box1 = root_box->AppendNew<TextBox>(L"foo");
  const auto& text_box2 = root_box->AppendNew<TextBox>(L"bar");

  EXPECT_FALSE(root_box->IsDescendantOf(*root_box));
  EXPECT_TRUE(text_box1->IsDescendantOf(*root_box));
  EXPECT_TRUE(text_box2->IsDescendantOf(*root_box));
  EXPECT_FALSE(text_box2->IsDescendantOf(*text_box1));
}

}  // namespace visuals
