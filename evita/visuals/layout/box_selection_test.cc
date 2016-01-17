// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/visuals/layout/box_selection.h"

#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/text.h"
#include "evita/visuals/layout/box_editor.h"
#include "evita/visuals/layout/box_selection_editor.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/layout/text_box.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(BoxSelectionTest, Basic) {
  const auto document = new Document();
  const auto node = new Text(document, L"foobar");
  const auto root_box = std::make_unique<RootBox>(*document);
  const auto text_box =
      std::make_unique<TextBox>(root_box.get(), node->data(), node);
  BoxSelection selection0;
  BoxSelection selection1;
  BoxSelectionEditor().Collapse(&selection1, text_box.get(), 1);
  BoxSelection selection2;
  BoxSelectionEditor().Collapse(&selection2, text_box.get(), 5);
  BoxSelectionEditor().ExtendTo(&selection2, text_box.get(), 3);

  EXPECT_TRUE(selection0.is_none());
  EXPECT_FALSE(selection0.is_caret());
  EXPECT_FALSE(selection0.is_range());

  EXPECT_FALSE(selection1.is_none());
  EXPECT_TRUE(selection1.is_caret());
  EXPECT_FALSE(selection1.is_range());

  EXPECT_EQ(text_box.get(), selection1.anchor_box());
  EXPECT_EQ(1, selection1.anchor_offset());

  EXPECT_FALSE(selection2.is_none());
  EXPECT_FALSE(selection2.is_caret());
  EXPECT_TRUE(selection2.is_range());

  EXPECT_EQ(text_box.get(), selection2.anchor_box());
  EXPECT_EQ(5, selection2.anchor_offset());
  EXPECT_EQ(text_box.get(), selection2.focus_box());
  EXPECT_EQ(3, selection2.focus_offset());

  BoxSelectionEditor().Clear(&selection2);
  EXPECT_TRUE(selection2.is_none());

  BoxEditor().RemoveAllChildren(root_box.get());
}

}  // namespace visuals
