// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/visuals/style/style_tree.h"

#include "base/observer_list.h"
#include "evita/visuals/css/mock_media.h"
#include "evita/visuals/css/properties.h"
#include "evita/visuals/css/selector_parser.h"
#include "evita/visuals/css/style.h"
#include "evita/visuals/css/style_builder.h"
#include "evita/visuals/css/style_sheet.h"
#include "evita/visuals/dom/document.h"
#include "evita/visuals/dom/element.h"
#include "evita/visuals/dom/node_tree_builder.h"
#include "evita/visuals/view/public/user_action_source.h"
#include "evita/visuals/view/public/view_lifecycle.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace visuals {

namespace {
css::Selector ParseSelector(base::StringPiece16 text) {
  return css::Selector::Parser().Parse(text);
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// StyleTreeTest
//
class StyleTreeTest : public ::testing::Test, public visuals::UserActionSource {
 protected:
  StyleTreeTest() = default;
  ~StyleTreeTest() override = default;

  const css::MockMedia& mock_media() const { return mock_media_; }

  void NotifyChangeHoveredNode(Node* hovered_node) {
    FOR_EACH_OBSERVER(UserActionSource::Observer, observers_,
                      DidChangeHoveredNode(hovered_node));
  }

 private:
  // visuals::UserActionSource
  void AddObserver(UserActionSource::Observer* observer) const final {
    observers_.AddObserver(observer);
  }

  void RemoveObserver(UserActionSource::Observer* observer) const final {
    observers_.RemoveObserver(observer);
  }

  css::MockMedia mock_media_;
  mutable base::ObserverList<UserActionSource::Observer> observers_;

  DISALLOW_COPY_AND_ASSIGN(StyleTreeTest);
};

TEST_F(StyleTreeTest, Basic) {
  const auto& document = NodeTreeBuilder()
                             .Begin(L"body")
                             .AddText(L"Hello world!")
                             .End(L"body")
                             .Build();
  const auto body = document->first_child()->as<Element>();
  ViewLifecycle lifecycle(*document, mock_media());
  ViewLifecycle::Scope(&lifecycle, ViewLifecycle::State::Started);
  StyleTree style_tree(&lifecycle, *this, {});
  style_tree.UpdateIfNeeded();

  EXPECT_EQ(style_tree.initial_style(), style_tree.ComputedStyleOf(*body));
  EXPECT_EQ(style_tree.initial_style(),
            style_tree.ComputedStyleOf(*body->first_child()));

  lifecycle.StartShutdown();
  lifecycle.FinishShutdown();
}

TEST_F(StyleTreeTest, ComputedStyleOfText) {
  const auto& kColorRed = css::Color(css::ColorValue(1, 0, 0));
  const auto& document =
      NodeTreeBuilder()
          .Begin(L"body")
          .SetInlineStyle(*css::StyleBuilder().SetColor(kColorRed).Build())
          .AddText(L"Hello world!")
          .End(L"body")
          .Build();
  const auto body = document->first_child()->as<Element>();
  ViewLifecycle lifecycle(*document, mock_media());
  ViewLifecycle::Scope(&lifecycle, ViewLifecycle::State::Started);
  StyleTree style_tree(&lifecycle, *this, {});
  style_tree.UpdateIfNeeded();

  EXPECT_EQ(kColorRed, style_tree.ComputedStyleOf(*body).color());
  EXPECT_EQ(kColorRed,
            style_tree.ComputedStyleOf(*body->first_child()).color());

  lifecycle.StartShutdown();
  lifecycle.FinishShutdown();
}

TEST_F(StyleTreeTest, Hover) {
  const auto& kColorRed = css::Color(css::ColorValue(1, 0, 0));
  const auto& kColorGreen = css::Color(css::ColorValue(0, 1, 0));
  auto* const style_sheet = new css::StyleSheet();
  style_sheet->AppendRule(
      ParseSelector(L":hover"),
      std::move(css::StyleBuilder().SetColor(kColorGreen).Build()));
  style_sheet->AppendRule(
      ParseSelector(L"foo"),
      std::move(css::StyleBuilder().SetColor(kColorRed).Build()));
  const auto& document = NodeTreeBuilder()
                             .Begin(L"body")
                             .Begin(L"foo")
                             .End(L"foo")
                             .End(L"body")
                             .Build();
  const auto body = document->first_child()->as<Element>();
  const auto foo = body->first_child()->as<Element>();
  ViewLifecycle lifecycle(*document, mock_media());
  ViewLifecycle::Scope(&lifecycle, ViewLifecycle::State::Started);
  StyleTree style_tree(&lifecycle, *this, {style_sheet});

  NotifyChangeHoveredNode(foo);
  style_tree.UpdateIfNeeded();
  EXPECT_EQ(kColorGreen, style_tree.ComputedStyleOf(*body).color())
      << "color comes from :hover";
  EXPECT_EQ(kColorGreen, style_tree.ComputedStyleOf(*foo).color())
      << "color comes from :hover";

  NotifyChangeHoveredNode(body);
  style_tree.UpdateIfNeeded();
  EXPECT_EQ(kColorGreen, style_tree.ComputedStyleOf(*body).color())
      << "color comes from :hover";
  EXPECT_EQ(kColorRed, style_tree.ComputedStyleOf(*foo).color())
      << "color comes from foo";

  lifecycle.StartShutdown();
  lifecycle.FinishShutdown();
}

TEST_F(StyleTreeTest, Inheritance) {
  const auto& kColorRed = css::Color(css::ColorValue(1, 0, 0));
  const auto& document =
      NodeTreeBuilder()
          .Begin(L"body")
          .SetInlineStyle(*css::StyleBuilder().SetColor(kColorRed).Build())
          .Begin(L"foo")
          .End(L"foo")
          .End(L"body")
          .Build();
  const auto body = document->first_child()->as<Element>();
  ViewLifecycle lifecycle(*document, mock_media());
  ViewLifecycle::Scope(&lifecycle, ViewLifecycle::State::Started);
  StyleTree style_tree(&lifecycle, *this, {});
  style_tree.UpdateIfNeeded();

  EXPECT_EQ(kColorRed, style_tree.ComputedStyleOf(*body).color());
  EXPECT_EQ(kColorRed,
            style_tree.ComputedStyleOf(*body->first_child()).color());

  lifecycle.StartShutdown();
  lifecycle.FinishShutdown();
}

}  // namespace visuals
