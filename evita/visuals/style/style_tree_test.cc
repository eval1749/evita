// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/style/style_tree.h"

#include "evita/visuals/css/mock_media.h"
#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_builder.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_tree_builder.h"
#include "evita/visuals/view/public/view_lifecycle.h"
#include "gtest/gtest.h"

namespace visuals {

//////////////////////////////////////////////////////////////////////
//
// StyleTreeTest
//
class StyleTreeTest : public ::testing::Test {
 protected:
  StyleTreeTest() = default;
  ~StyleTreeTest() override = default;

  const css::MockMedia& mock_media() const { return mock_media_; }

 private:
  css::MockMedia mock_media_;

  DISALLOW_COPY_AND_ASSIGN(StyleTreeTest);
};

TEST_F(StyleTreeTest, Basic) {
  const auto& document = NodeTreeBuilder()
                             .Begin(L"body")
                             .AddText(L"Hello world!")
                             .End(L"body")
                             .Build();
  const auto body = document->first_child()->as<Element>();
  ViewLifecycle lifecycle(*document);
  StyleTree style_tree(&lifecycle, mock_media(), {});
  style_tree.UpdateIfNeeded();

  EXPECT_EQ(style_tree.initial_style(), style_tree.ComputedStyleOf(*body));
  EXPECT_EQ(style_tree.initial_style(),
            style_tree.ComputedStyleOf(*body->first_child()));

  lifecycle.StartShutdown();
  lifecycle.FinishShutdown();
}

TEST_F(StyleTreeTest, Inheritance) {
  const auto& kColorRed = css::Color(1, 0, 0);
  const auto& document =
      NodeTreeBuilder()
          .Begin(L"body")
          .SetInlineStyle(*css::StyleBuilder().SetColor(kColorRed).Build())
          .Begin(L"foo")
          .End(L"foo")
          .End(L"body")
          .Build();
  const auto body = document->first_child()->as<Element>();
  ViewLifecycle lifecycle(*document);
  StyleTree style_tree(&lifecycle, mock_media(), {});
  style_tree.UpdateIfNeeded();

  EXPECT_EQ(kColorRed, style_tree.ComputedStyleOf(*body).color());
  EXPECT_EQ(kColorRed,
            style_tree.ComputedStyleOf(*body->first_child()).color());

  lifecycle.StartShutdown();
  lifecycle.FinishShutdown();
}

TEST_F(StyleTreeTest, ComputedStyleOfText) {
  const auto& kColorRed = css::Color(1, 0, 0);
  const auto& document =
      NodeTreeBuilder()
          .Begin(L"body")
          .SetInlineStyle(*css::StyleBuilder().SetColor(kColorRed).Build())
          .AddText(L"Hello world!")
          .End(L"body")
          .Build();
  const auto body = document->first_child()->as<Element>();
  ViewLifecycle lifecycle(*document);
  StyleTree style_tree(&lifecycle, mock_media(), {});
  style_tree.UpdateIfNeeded();

  EXPECT_EQ(kColorRed, style_tree.ComputedStyleOf(*body).color());
  EXPECT_EQ(kColorRed,
            style_tree.ComputedStyleOf(*body->first_child()).color());

  lifecycle.StartShutdown();
  lifecycle.FinishShutdown();
}

}  // namespace visuals
