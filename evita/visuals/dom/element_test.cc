// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "evita/visuals/dom/element.h"

#include "base/strings/string_util.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/document_observer.h"
#include "evita/visuals/dom/node_tree_builder.h"
#include "evita/visuals/dom/node_editor.h"
#include "gtest/gtest.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// ElementTest
//
class ElementTest : public ::testing::Test, public DocumentObserver {
 protected:
  ElementTest() = default;
  ~ElementTest() override = default;

  const std::vector<base::string16> added_classes() const {
    return added_classes_;
  }
  const std::vector<base::string16> removed_classes() const {
    return removed_classes_;
  }

 private:
  // DocumentObserver
  void DidAddClass(const ElementNode& element, const base::string16& name);
  void DidRemoveClass(const ElementNode& element, const base::string16& name);

  std::vector<base::string16> added_classes_;
  std::vector<base::string16> removed_classes_;

  DISALLOW_COPY_AND_ASSIGN(ElementTest);
};

void ElementTest::DidAddClass(const ElementNode& element,
                              const base::string16& name) {
  added_classes_.push_back(name);
}

void ElementTest::DidRemoveClass(const ElementNode& element,
                                 const base::string16& name) {
  removed_classes_.push_back(name);
}

TEST_F(ElementTest, class_list) {
  const auto document = NodeTreeBuilder()
                            .Begin(L"test")
                            .ClassList({L"foo", L"bar"})
                            .End(L"test")
                            .Build();
  document->AddObserver(this);
  const auto element = document->first_child()->as<Element>();

  EXPECT_EQ(L"foo bar", base::JoinString(element->class_list(), L" "));

  NodeEditor().AddClass(element, L"baz");
  EXPECT_EQ(L"foo bar baz", base::JoinString(element->class_list(), L" "));
  EXPECT_EQ(L"baz", base::JoinString(added_classes(), L" "));

  NodeEditor().AddClass(element, L"bar");
  EXPECT_EQ(L"foo bar baz", base::JoinString(element->class_list(), L" "))
      << "class 'bar' is already in list.";

  NodeEditor().RemoveClass(element, L"bar");
  EXPECT_EQ(L"foo baz", base::JoinString(element->class_list(), L" "));
  EXPECT_EQ(L"bar", base::JoinString(removed_classes(), L" "));

  NodeEditor().RemoveClass(element, L"quux");
  EXPECT_EQ(L"foo baz", base::JoinString(element->class_list(), L" "))
      << "Removing non-existing class doesn't affect class list.";

  EXPECT_EQ(L"bar", base::JoinString(removed_classes(), L" "));
}

}  // namespace visuals
