// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>
#include <utility>

#include "evita/visuals/model/root_box.h"

#include "evita/visuals/model/block_flow_box.h"
#include "evita/visuals/model/box_editor.h"
#include "evita/visuals/model/descendants_or_self.h"
#include "evita/visuals/model/simple_box_tree_builder.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(RootBoxTest, GetBoxById) {
  const auto& root = SimpleBoxTreeBuilder()
                         .Add<BlockFlowBox>(L"block1")
                         .Begin<BlockFlowBox>(L"block2")
                         .Add<BlockFlowBox>(L"block3")
                         .Add<BlockFlowBox>(L"block4")
                         .End<BlockFlowBox>()
                         .Build();

  std::map<base::string16, Box*> id_map;
  for (const auto& box : Box::DescendantsOrSelf(*root)) {
    if (box->id().empty())
      continue;
    id_map.insert(std::make_pair(box->id(), box));
  }

  EXPECT_EQ(id_map.find(L"block1")->second, root->GetBoxById(L"block1"));
  EXPECT_EQ(id_map.find(L"block3")->second, root->GetBoxById(L"block3"));
  EXPECT_EQ(nullptr, root->GetBoxById(L"not exist"));

  BoxEditor().RemoveChild(root.get(), root->GetBoxById(L"block2"));
  EXPECT_EQ(nullptr, root->GetBoxById(L"block2"));
}

}  // namespace visuals
