// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/layout/box_tree_builder.h"

#include "evita/visuals/css/mock_media.h"
#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_builder.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_tree_builder.h"
#include "evita/visuals/model/block_flow_box.h"
#include "evita/visuals/model/inline_flow_box.h"
#include "evita/visuals/model/root_box.h"
#include "evita/visuals/style/style_resolver.h"
#include "gtest/gtest.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// BoxTreeBuilderTest
//
class BoxTreeBuilderTest : public ::testing::Test {
 protected:
  BoxTreeBuilderTest() = default;
  ~BoxTreeBuilderTest() override = default;

  const css::MockMedia& mock_media() const { return mock_media_; }

 private:
  css::MockMedia mock_media_;

  DISALLOW_COPY_AND_ASSIGN(BoxTreeBuilderTest);
};

TEST_F(BoxTreeBuilderTest, Basic) {
  const auto& document = NodeTreeBuilder()
                             .Begin(L"body")
                             .AddText(L"Hello world!")
                             .End(L"body")
                             .Build();
  BoxTreeBuilder builder(*document, mock_media(), {});
  const auto& root_box = builder.Build();
  EXPECT_TRUE(root_box->first_child()->is<InlineFlowBox>());
}

TEST_F(BoxTreeBuilderTest, Flow) {
  const auto& document = NodeTreeBuilder()
                             .Begin(L"body")
                             .AddText(L"foo ")
                             .AddText(L"bar")
                             .End(L"body")
                             .Build();
  BoxTreeBuilder builder(*document, mock_media(), {});
  const auto& root_box = builder.Build();
  EXPECT_TRUE(root_box->first_child()->is<InlineFlowBox>());
}

}  // namespace visuals
