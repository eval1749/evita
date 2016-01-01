// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>
#include <utility>

#include "evita/visuals/model/root_box.h"

#include "evita/visuals/model/block_box.h"
#include "evita/visuals/model/box_builder.h"
#include "evita/visuals/model/box_editor.h"
#include "evita/visuals/model/box_tree_builder.h"
#include "evita/visuals/model/descendants_or_self.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(RootBoxTest, GetBoxById) {
  const auto& root =
      BoxTreeBuilder()
          .Append(BoxBuilder::New<BlockBox>(L"block1").Finish())
          .Append(BoxBuilder::New<BlockBox>(L"block2")
                      .Append(BoxBuilder::New<BlockBox>(L"block3").Finish())
                      .Append(BoxBuilder::New<BlockBox>(L"block4").Finish())
                      .Finish())
          .Finish();

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
