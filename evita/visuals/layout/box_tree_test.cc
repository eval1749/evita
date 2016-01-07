// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/layout/box_tree.h"

#include "evita/visuals/css/mock_media.h"
#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_builder.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_tree_builder.h"
#include "evita/visuals/model/block_flow_box.h"
#include "evita/visuals/model/inline_box.h"
#include "evita/visuals/model/inline_flow_box.h"
#include "evita/visuals/model/root_box.h"
#include "evita/visuals/style/style_tree.h"
#include "gtest/gtest.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// BoxTreeTest
//
class BoxTreeTest : public ::testing::Test {
 protected:
  BoxTreeTest() = default;
  ~BoxTreeTest() override = default;

  const css::MockMedia& mock_media() const { return mock_media_; }

 private:
  css::MockMedia mock_media_;

  DISALLOW_COPY_AND_ASSIGN(BoxTreeTest);
};

TEST_F(BoxTreeTest, Basic) {
  const auto& document = NodeTreeBuilder()
                             .Begin(L"body")
                             .AddText(L"Hello world!")
                             .End(L"body")
                             .Build();

  StyleTree style_tree(*document, mock_media(), {});
  style_tree.UpdateIfNeeded();
  BoxTree tree(*document, style_tree);
  tree.UpdateIfNeeded();
  const auto root_box = tree.root_box();
  EXPECT_TRUE(root_box->first_child()->is<InlineBox>());
}

TEST_F(BoxTreeTest, Flow) {
  const auto& document = NodeTreeBuilder()
                             .Begin(L"body")
                             .AddText(L"foo ")
                             .AddText(L"bar")
                             .End(L"body")
                             .Build();
  StyleTree style_tree(*document, mock_media(), {});
  style_tree.UpdateIfNeeded();
  BoxTree tree(*document, style_tree);
  tree.UpdateIfNeeded();
  const auto root_box = tree.root_box();
  EXPECT_TRUE(root_box->first_child()->is<InlineBox>());
}

}  // namespace visuals
