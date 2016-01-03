// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/style/style_resolver.h"

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
// StyleResolverTest
//
class StyleResolverTest : public ::testing::Test {
 protected:
  StyleResolverTest() = default;
  ~StyleResolverTest() override = default;

  const css::MockMedia& mock_media() const { return mock_media_; }

 private:
  css::MockMedia mock_media_;

  DISALLOW_COPY_AND_ASSIGN(StyleResolverTest);
};

TEST_F(StyleResolverTest, Basic) {
  const auto& document = NodeTreeBuilder()
                             .Begin(L"body")
                             .AddText(L"Hello world!")
                             .End(L"body")
                             .Build();
  const auto body = document->first_child()->as<Element>();
  StyleResolver resolver(*document, mock_media());
  EXPECT_EQ(resolver.default_style(), resolver.ResolveFor(*body));
  EXPECT_EQ(resolver.default_style(),
            resolver.ResolveFor(*body->first_child()));
}

TEST_F(StyleResolverTest, Inheritance) {
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
  StyleResolver resolver(*document, mock_media());
  EXPECT_EQ(kColorRed, resolver.ResolveFor(*body).color());
  EXPECT_EQ(kColorRed, resolver.ResolveFor(*body->first_child()).color());
}

TEST_F(StyleResolverTest, ResolveForText) {
  const auto& kColorRed = css::Color(1, 0, 0);
  const auto& document =
      NodeTreeBuilder()
          .Begin(L"body")
          .SetStyle(*css::StyleBuilder().SetColor(kColorRed).Build())
          .AddText(L"Hello world!")
          .End(L"body")
          .Build();
  const auto body = document->first_child()->as<Element>();
  StyleResolver resolver(*document, mock_media());
  EXPECT_EQ(kColorRed, resolver.ResolveFor(*body).color());
  EXPECT_EQ(kColorRed, resolver.ResolveFor(*body->first_child()).color());
}

}  // namespace visuals
