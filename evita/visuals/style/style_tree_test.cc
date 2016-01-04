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
  StyleTree resolver(*document, mock_media(), {});
  resolver.UpdateIfNeeded();

  EXPECT_EQ(resolver.initial_style(), resolver.ComputedStyleOf(*body));
  EXPECT_EQ(resolver.initial_style(),
            resolver.ComputedStyleOf(*body->first_child()));
}

TEST_F(StyleTreeTest, Inheritance) {
  const auto& kColorRed = css::Color(1, 0, 0);
  const auto& document =
      NodeTreeBuilder()
          .Begin(L"body")
          .SetStyle(*css::StyleBuilder().SetColor(kColorRed).Build())
          .Begin(L"foo")
          .End(L"foo")
          .End(L"body")
          .Build();
  const auto body = document->first_child()->as<Element>();
  StyleTree resolver(*document, mock_media(), {});
  resolver.UpdateIfNeeded();

  EXPECT_EQ(kColorRed, resolver.ComputedStyleOf(*body).color());
  EXPECT_EQ(kColorRed, resolver.ComputedStyleOf(*body->first_child()).color());
}

TEST_F(StyleTreeTest, ComputedStyleOfText) {
  const auto& kColorRed = css::Color(1, 0, 0);
  const auto& document =
      NodeTreeBuilder()
          .Begin(L"body")
          .SetStyle(*css::StyleBuilder().SetColor(kColorRed).Build())
          .AddText(L"Hello world!")
          .End(L"body")
          .Build();
  const auto body = document->first_child()->as<Element>();
  StyleTree resolver(*document, mock_media(), {});
  resolver.UpdateIfNeeded();

  EXPECT_EQ(kColorRed, resolver.ComputedStyleOf(*body).color());
  EXPECT_EQ(kColorRed, resolver.ComputedStyleOf(*body->first_child()).color());
}

}  // namespace visuals
