// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/dom/selection.h"

#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/selection_editor.h"
#include "evita/visuals/dom/text.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(SelectionTest, Basic) {
  const auto document = new Document();
  const auto node = new Text(document, L"foobar");
  Selection selection0;
  Selection selection1;
  SelectionEditor().Collapse(&selection1, node, 1);
  Selection selection2;
  SelectionEditor().Collapse(&selection2, node, 5);
  SelectionEditor().ExtendTo(&selection2, node, 3);

  EXPECT_TRUE(selection0.is_none());
  EXPECT_FALSE(selection0.is_caret());
  EXPECT_FALSE(selection0.is_range());

  EXPECT_FALSE(selection1.is_none());
  EXPECT_TRUE(selection1.is_caret());
  EXPECT_FALSE(selection1.is_range());

  EXPECT_EQ(node, selection1.anchor_node());
  EXPECT_EQ(1, selection1.anchor_offset());

  EXPECT_FALSE(selection2.is_none());
  EXPECT_FALSE(selection2.is_caret());
  EXPECT_TRUE(selection2.is_range());

  EXPECT_EQ(node, selection2.anchor_node());
  EXPECT_EQ(5, selection2.anchor_offset());
  EXPECT_EQ(3, selection2.focus_offset());

  SelectionEditor().Clear(&selection2);
  EXPECT_TRUE(selection2.is_none());
}

}  // namespace visuals
