// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "evita/visuals/dom/element.h"

#include "base/strings/string_util.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/node_tree_builder.h"
#include "evita/visuals/dom/node_editor.h"
#include "gtest/gtest.h"

namespace visuals {

TEST(ElementTest, class_list) {
  const auto document = NodeTreeBuilder()
                            .Begin(L"test")
                            .ClassList({L"foo", L"bar"})
                            .End(L"test")
                            .Build();
  const auto element = document->first_child()->as<Element>();

  EXPECT_EQ(L"foo bar", base::JoinString(element->class_list(), L" "));

  NodeEditor().AddClass(element, L"baz");
  EXPECT_EQ(L"foo bar baz", base::JoinString(element->class_list(), L" "));

  NodeEditor().AddClass(element, L"bar");
  EXPECT_EQ(L"foo bar baz", base::JoinString(element->class_list(), L" "))
      << "class 'bar' is already in list.";

  NodeEditor().RemoveClass(element, L"bar");
  EXPECT_EQ(L"foo baz", base::JoinString(element->class_list(), L" "));

  NodeEditor().RemoveClass(element, L"quux");
  EXPECT_EQ(L"foo baz", base::JoinString(element->class_list(), L" "))
      << "Removing non-existing class doesn't affect class list.";
}

}  // namespace visuals
