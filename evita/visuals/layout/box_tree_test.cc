// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "evita/visuals/layout/box_tree.h"

#include "base/macros.h"
#include "base/strings/utf_string_conversions.h"
#include "evita/visuals/css/mock_media.h"
#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_builder.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_tree_builder.h"
#include "evita/visuals/layout/descendants_or_self.h"
#include "evita/visuals/layout/root_box.h"
#include "evita/visuals/layout/text_box.h"
#include "evita/visuals/style/style_tree.h"
#include "evita/visuals/view/public/selection.h"
#include "evita/visuals/view/public/view_lifecycle.h"
#include "gtest/gtest.h"

namespace visuals {

namespace {

//////////////////////////////////////////////////////////////////////
//
// BoxStringify
//
class BoxStringify final {
 public:
  BoxStringify() = default;
  ~BoxStringify() = default;

  std::string ToString(const Box& box);

 private:
  void Visit(const Box& box);

  std::ostringstream ostream_;

  DISALLOW_COPY_AND_ASSIGN(BoxStringify);
};

std::string BoxStringify::ToString(const Box& box) {
  Visit(box);
  return ostream_.str();
}

void BoxStringify::Visit(const Box& box) {
  if (const auto container_box = box.as<ContainerBox>()) {
    ostream_ << box.class_name() << ':' << box.display() << '(';
    auto delimiter = "";
    for (const auto& child : container_box->child_boxes()) {
      ostream_ << delimiter;
      Visit(*child);
      delimiter = " ";
    }
    ostream_ << ')';
    return;
  }
  if (const auto text_box = box.as<TextBox>()) {
    ostream_ << '\'' << text_box->data() << '\'';
    return;
  }
  ostream_ << box.class_name();
}

std::string BoxTreeToString(const BoxTree& box_tree) {
  return BoxStringify().ToString(*box_tree.root_box());
}

//////////////////////////////////////////////////////////////////////
//
// MockView
//
class MockView final {
 public:
  MockView(const Document& document, const css::Media& media);
  ~MockView();

  const BoxTree& box_tree() const { return *box_tree_; }

 private:
  std::unique_ptr<ViewLifecycle> lifecycle_;
  std::unique_ptr<Selection> selection_;

  // |StyleTree| takes |ViewLifecycle| and |css::Media|.
  std::unique_ptr<StyleTree> style_tree_;

  // |BoxTree| takes |ViewLifecycle|, |Selection| and |StyleTree|.
  std::unique_ptr<BoxTree> box_tree_;

  DISALLOW_COPY_AND_ASSIGN(MockView);
};

MockView::MockView(const Document& document, const css::Media& media)
    : lifecycle_(new ViewLifecycle(document)),
      selection_(new Selection(document, media)),
      style_tree_(new StyleTree(lifecycle_.get(), media, {})),
      box_tree_(new BoxTree(lifecycle_.get(), *selection_, *style_tree_)) {
  style_tree_->UpdateIfNeeded();
  box_tree_->UpdateIfNeeded();
}

MockView::~MockView() {
  lifecycle_->StartShutdown();
  box_tree_.reset();
  lifecycle_->FinishShutdown();
}

}  // namespace

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
  MockView view(*document, mock_media());
  EXPECT_EQ("RootBox:inline(FlowBox:inline('Hello world!'))",
            BoxTreeToString(view.box_tree()));
}

TEST_F(BoxTreeTest, FlowAnonymous) {
  auto display_block =
      css::StyleBuilder().SetDisplay(css::Display::Block()).Build();
  const auto& document = NodeTreeBuilder()
                             .Begin(L"div")
                             .SetInlineStyle(*display_block)
                             .AddText(L"foo")
                             .Begin(L"p")
                             .SetInlineStyle(*display_block)
                             .AddText(L"bar")
                             .End(L"p")
                             .AddText(L"baz")
                             .End(L"div")
                             .Build();
  MockView view(*document, mock_media());
  EXPECT_EQ(
      "RootBox:inline(FlowBox:block(FlowBox:block('foo') FlowBox:block('bar') "
      "FlowBox:block('baz')))",
      BoxTreeToString(view.box_tree()));
}

TEST_F(BoxTreeTest, FlowInline) {
  const auto& document = NodeTreeBuilder()
                             .Begin(L"body")
                             .AddText(L"foo ")
                             .AddText(L"bar")
                             .End(L"body")
                             .Build();
  MockView view(*document, mock_media());
  EXPECT_EQ("RootBox:inline(FlowBox:inline('foo ' 'bar'))",
            BoxTreeToString(view.box_tree()));
}

}  // namespace visuals
