// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <map>
#include <utility>

#include "evita/visuals/dom/document.h"

#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/descendants_or_self.h"
#include "evita/visuals/dom/node_editor.h"
#include "evita/visuals/dom/node_tree_builder.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(DocumentTest, GetElementById) {
  const auto& document = NodeTreeBuilder()
                             .Begin(L"block", L"block1")
                             .End(L"block")
                             .Begin(L"block", L"block2")
                             .Begin(L"block", L"block3")
                             .End(L"block")
                             .Begin(L"block", L"block4")
                             .End(L"block")
                             .End(L"block")
                             .Build();

  std::map<base::string16, Node*> id_map;
  for (const auto& node : Node::DescendantsOrSelf(*document)) {
    if (node->id().empty())
      continue;
    id_map.insert(std::make_pair(node->id(), node));
  }

  EXPECT_EQ(id_map.find(L"block1")->second,
            document->GetElementById(L"block1"));
  EXPECT_EQ(id_map.find(L"block3")->second,
            document->GetElementById(L"block3"));
  EXPECT_EQ(nullptr, document->GetElementById(L"not exist"));

  NodeEditor().RemoveChild(document, document->GetElementById(L"block2"));
  EXPECT_EQ(nullptr, document->GetElementById(L"block2"));
}

}  // namespace visuals
