// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/view/public/selection_model.h"

#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/text.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(SelectionModelTest, Basic) {
  const auto document = new Document();
  const auto node = new Text(document, L"foobar");
  SelectionModel selection0;
  SelectionModel selection1;
  selection1.Collapse(node, 1);
  SelectionModel selection2;
  selection2.Collapse(node, 5);
  selection2.ExtendTo(node, 3);

  EXPECT_TRUE(selection0.is_none());
  EXPECT_FALSE(selection0.is_caret());
  EXPECT_FALSE(selection0.is_range());

  EXPECT_FALSE(selection1.is_none());
  EXPECT_TRUE(selection1.is_caret());
  EXPECT_FALSE(selection1.is_range());

  EXPECT_EQ(*node, selection1.focus_node());
  EXPECT_EQ(1, selection1.focus_offset());

  EXPECT_FALSE(selection2.is_none());
  EXPECT_FALSE(selection2.is_caret());
  EXPECT_TRUE(selection2.is_range());

  EXPECT_EQ(*node, selection2.anchor_node());
  EXPECT_EQ(5, selection2.anchor_offset());
  EXPECT_EQ(*node, selection2.focus_node());
  EXPECT_EQ(3, selection2.focus_offset());

  selection2.Clear();
  EXPECT_TRUE(selection2.is_none());
}

}  // namespace visuals
