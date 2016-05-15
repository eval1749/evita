// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <set>
#include <sstream>
#include <vector>

#include "base/strings/string16.h"
#include "evita/visuals/css/selector_builder.h"
#include "evita/visuals/css/selector_parser.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace visuals {
namespace css {

namespace {
base::string16 AsParseError(base::StringPiece16 text) {
  Selector::Parser parser;
  parser.Parse(text);
  return std::move(parser.error().as_string());
}

Selector AsSelector(base::StringPiece16 tag_name,
                    base::StringPiece16 id,
                    const std::vector<base::StringPiece16>& class_names) {
  Selector::Builder builder;
  if (!tag_name.empty())
    builder.SetTagName(tag_name);
  if (!id.empty())
    builder.SetId(id);
  for (auto class_name : class_names)
    builder.AddClass(class_name);
  return std::move(builder.Build());
}

Selector AsSelector(base::StringPiece16 tag_name,
                    const std::vector<base::StringPiece16>& class_names) {
  return std::move(AsSelector(tag_name, L"", class_names));
}

Selector AsSelector(base::StringPiece16 tag_name) {
  return std::move(AsSelector(tag_name, L"", {}));
}

std::string AsString(const std::vector<Selector>& selectors) {
  std::ostringstream ostream;
  auto delimiter = "{";
  for (const auto& selector : selectors) {
    ostream << delimiter << selector;
    delimiter = ", ";
  }
  ostream << '}';
  return std::move(ostream.str());
}

const Selector& MoreSpecific(const Selector& selector1,
                             const Selector& selector2) {
  return selector1.IsMoreSpecific(selector2) ? selector1 : selector2;
}

Selector Parse(base::StringPiece16 text) {
  return std::move(Selector::Parser().Parse(text));
}

}  // namespace

TEST(CssSelctorTest, Equals) {
  Selector selector0;
  const auto& selector1 = Parse(L"foo");
  const auto& selector11 = Parse(L"foo");
  const auto& selector2 = Parse(L"bar");
  const auto& selector22 = Parse(L"bar");
  const auto& selector3 = Parse(L"foo.c1");
  const auto& selector33 = Parse(L"foo.c1");
  const auto& selector4 = Parse(L"foo#bar.c1");
  const auto& selector44 = Parse(L"foo#bar.c1");
  const auto& selector5 = Parse(L"foo#bar.c1.c2");
  const auto& selector55 = Parse(L"foo#bar.c1.c2");

  EXPECT_TRUE(selector0 == selector0);
  EXPECT_TRUE(selector0 != selector1);
  EXPECT_TRUE(selector0 != selector11);
  EXPECT_TRUE(selector0 != selector2);
  EXPECT_TRUE(selector0 != selector22);
  EXPECT_TRUE(selector0 != selector3);
  EXPECT_TRUE(selector0 != selector33);
  EXPECT_TRUE(selector0 != selector4);
  EXPECT_TRUE(selector0 != selector44);
  EXPECT_TRUE(selector0 != selector5);
  EXPECT_TRUE(selector0 != selector55);
  EXPECT_FALSE(selector0 != selector0);

  EXPECT_FALSE(selector1 == selector0);
  EXPECT_TRUE(selector1 == selector1);
  EXPECT_TRUE(selector1 == selector11);
  EXPECT_TRUE(selector1 != selector2);
  EXPECT_TRUE(selector1 != selector22);
  EXPECT_TRUE(selector1 != selector3);
  EXPECT_TRUE(selector1 != selector33);
  EXPECT_TRUE(selector1 != selector4);
  EXPECT_TRUE(selector1 != selector44);
  EXPECT_TRUE(selector1 != selector5);
  EXPECT_TRUE(selector1 != selector55);
  EXPECT_FALSE(selector1 != selector1);

  EXPECT_TRUE(selector2 == selector22);
  EXPECT_TRUE(selector3 == selector33);
  EXPECT_TRUE(selector4 == selector44);
  EXPECT_TRUE(selector5 == selector55);
}

TEST(CssSelctorTest, IsMoreSpecific) {
  EXPECT_EQ(Parse(L"*"), MoreSpecific(Parse(L"*"), Parse(L"*")));
  EXPECT_EQ(Parse(L"foo"), MoreSpecific(Parse(L"foo"), Parse(L"*")));
  EXPECT_EQ(Parse(L"foo"), MoreSpecific(Parse(L"foo"), Parse(L"foo")));
  EXPECT_EQ(Parse(L"#bar"), MoreSpecific(Parse(L"foo"), Parse(L"#bar")));
  EXPECT_EQ(Parse(L"foo#bar"), MoreSpecific(Parse(L"foo#bar"), Parse(L"#bar")));
  EXPECT_EQ(Parse(L"foo#bar"),
            MoreSpecific(Parse(L"foo#bar"), Parse(L"foo.c1")));
  EXPECT_EQ(Parse(L"foo.c1"), MoreSpecific(Parse(L"foo.c1"), Parse(L"foo")));
  EXPECT_EQ(Parse(L"foo.c2"), MoreSpecific(Parse(L"foo.c1"), Parse(L"foo.c2")));
  EXPECT_EQ(Parse(L"foo.c1.c2"),
            MoreSpecific(Parse(L"foo.c1.c2"), Parse(L"foo.c2")));
}

TEST(CssSelctorTest, IsSubsetOf) {
  EXPECT_TRUE(Parse(L"*").IsSubsetOf(Parse(L"*")));
  EXPECT_FALSE(Parse(L"*").IsSubsetOf(Parse(L"*#bar")));
  EXPECT_FALSE(Parse(L"*").IsSubsetOf(Parse(L"*.c1")));
  EXPECT_FALSE(Parse(L"*").IsSubsetOf(Parse(L"foo")));
  EXPECT_FALSE(Parse(L"*").IsSubsetOf(Parse(L"foo#bar")));
  EXPECT_FALSE(Parse(L"*").IsSubsetOf(Parse(L"foo.c1")));
  EXPECT_FALSE(Parse(L"*").IsSubsetOf(Parse(L"foo.c1.c2")));
  EXPECT_FALSE(Parse(L"*").IsSubsetOf(Parse(L"foo:hover")));

  EXPECT_TRUE(Parse(L"foo").IsSubsetOf(Parse(L"*")));
  EXPECT_FALSE(Parse(L"foo").IsSubsetOf(Parse(L"*#bar")));
  EXPECT_FALSE(Parse(L"foo").IsSubsetOf(Parse(L"*.c1")));
  EXPECT_TRUE(Parse(L"foo").IsSubsetOf(Parse(L"foo")));
  EXPECT_FALSE(Parse(L"foo").IsSubsetOf(Parse(L"foo#bar")));
  EXPECT_FALSE(Parse(L"foo").IsSubsetOf(Parse(L"foo.c1")));
  EXPECT_FALSE(Parse(L"foo").IsSubsetOf(Parse(L"foo:hover")));

  EXPECT_TRUE(Parse(L"foo#bar").IsSubsetOf(Parse(L"*")));
  EXPECT_TRUE(Parse(L"foo#bar").IsSubsetOf(Parse(L"*#bar")));
  EXPECT_FALSE(Parse(L"foo#bar").IsSubsetOf(Parse(L"*.c1")));
  EXPECT_TRUE(Parse(L"foo#bar").IsSubsetOf(Parse(L"foo")));
  EXPECT_TRUE(Parse(L"foo#bar").IsSubsetOf(Parse(L"foo#bar")));
  EXPECT_FALSE(Parse(L"foo#bar").IsSubsetOf(Parse(L"foo.c1")));
  EXPECT_FALSE(Parse(L"foo#bar").IsSubsetOf(Parse(L"foo.c1.c2")));
  EXPECT_FALSE(Parse(L"foo#bar").IsSubsetOf(Parse(L"foo:hover")));

  EXPECT_TRUE(Parse(L"foo.c1").IsSubsetOf(Parse(L"*")));
  EXPECT_FALSE(Parse(L"foo.c1").IsSubsetOf(Parse(L"*#bar")));
  EXPECT_TRUE(Parse(L"foo.c1").IsSubsetOf(Parse(L"*.c1")));
  EXPECT_TRUE(Parse(L"foo.c1").IsSubsetOf(Parse(L"foo")));
  EXPECT_FALSE(Parse(L"foo.c1").IsSubsetOf(Parse(L"foo#bar")));
  EXPECT_TRUE(Parse(L"foo.c1").IsSubsetOf(Parse(L"foo.c1")));
  EXPECT_FALSE(Parse(L"foo.c1").IsSubsetOf(Parse(L"foo.c1.c2")));
  EXPECT_FALSE(Parse(L"foo.c1").IsSubsetOf(Parse(L"foo:hover")));

  EXPECT_TRUE(Parse(L"foo.c1.c2").IsSubsetOf(Parse(L"*")));
  EXPECT_FALSE(Parse(L"foo.c1.c2").IsSubsetOf(Parse(L"*#bar")));
  EXPECT_TRUE(Parse(L"foo.c1.c2").IsSubsetOf(Parse(L"*.c1")));
  EXPECT_TRUE(Parse(L"foo.c1.c2").IsSubsetOf(Parse(L"foo")));
  EXPECT_FALSE(Parse(L"foo.c1.c2").IsSubsetOf(Parse(L"foo#bar")));
  EXPECT_TRUE(Parse(L"foo.c1.c2").IsSubsetOf(Parse(L"foo.c1")));
  EXPECT_TRUE(Parse(L"foo.c1.c2").IsSubsetOf(Parse(L"foo.c1.c2")));
  EXPECT_FALSE(Parse(L"foo.c1.c2").IsSubsetOf(Parse(L"foo:hover")));
}

TEST(CssSelctorTest, Less) {
  Selector selector0;
  const auto& selector1 = Parse(L"foo");
  const auto& selector2 = Parse(L"bar");
  const auto& selector3 = Parse(L"foo#bar");
  const auto& selector4 = Parse(L"foo#bar.c1");
  const auto& selector5 = Parse(L"foo#bar.c1.c2");

  std::set<Selector> set;
  set.insert(selector0);
  set.insert(selector1);
  set.insert(selector2);
  set.insert(selector3);
  set.insert(selector4);
  set.insert(selector5);

  const auto& actual = std::vector<Selector>(set.begin(), set.end());
  ASSERT_EQ(6u, actual.size()) << AsString(actual);
  EXPECT_EQ(selector2, actual[0]);
  EXPECT_EQ(selector5, actual[1]);
  EXPECT_EQ(selector4, actual[2]);
  EXPECT_EQ(selector3, actual[3]);
  EXPECT_EQ(selector1, actual[4]);
  EXPECT_EQ(selector0, actual[5]);
}

TEST(CssSelctorTest, Parser) {
  EXPECT_EQ(Selector(), Parse(L""));
  EXPECT_EQ(Selector(), Parse(L"*"));
  EXPECT_EQ(AsSelector(L"", L"bar", {}), Parse(L"*#bar"));
  EXPECT_EQ(AsSelector(L"", {L"c1"}), Parse(L"*.c1"));
  EXPECT_EQ(AsSelector(L"", {L":hover"}), Parse(L"*:hover"));
  EXPECT_EQ(AsSelector(L"foo"), Parse(L"foo"));
  EXPECT_EQ(AsSelector(L"foo", L"bar", {}), Parse(L"foo#bar"));
  EXPECT_EQ(AsSelector(L"foo", {L"a"}), Parse(L"foo.a"));
  EXPECT_EQ(AsSelector(L"foo", {L":hover"}), Parse(L"foo:hover"));
  EXPECT_EQ(AsSelector(L"foo", L"bar", {L"c1"}), Parse(L"foo#bar.c1"));
  EXPECT_EQ(AsSelector(L"foo", L"bar", {L"c1", L"c2"}),
            Parse(L"foo#bar.c1.c2"));
  EXPECT_EQ(AsSelector(L"::selector"), Parse(L"::selector"));
  EXPECT_EQ(AsSelector(L"::selector", {L":active"}),
            Parse(L"::selector:active"));
}

TEST(CssSelctorTest, ParseError) {
  EXPECT_EQ(L"Bad tag name", AsParseError(L"!"));

  EXPECT_EQ(L"Bad tag name", AsParseError(L"*a"));
  EXPECT_EQ(L"Empty id", AsParseError(L"*#"));
  EXPECT_EQ(L"Empty class", AsParseError(L"*."));
  EXPECT_EQ(L"Empty pseudo-class", AsParseError(L"*:"));

  EXPECT_EQ(L"Empty id", AsParseError(L"#"));
  EXPECT_EQ(L"Empty id", AsParseError(L"#."));
  EXPECT_EQ(L"Empty id", AsParseError(L"#:"));
  EXPECT_EQ(L"Bad id", AsParseError(L"#!"));
  EXPECT_EQ(L"Bad id", AsParseError(L"##"));

  EXPECT_EQ(L"Empty class", AsParseError(L":"));
  EXPECT_EQ(L"Empty pseudo-element", AsParseError(L"::"));
  EXPECT_EQ(L"Empty pseudo-element", AsParseError(L":::"));
  EXPECT_EQ(L"Empty pseudo-element", AsParseError(L"::#"));
  EXPECT_EQ(L"Empty pseudo-element", AsParseError(L"::."));
  EXPECT_EQ(L"Bad tag name", AsParseError(L"::!"));

  EXPECT_EQ(L"Bad tag name", AsParseError(L"foo!"));
  EXPECT_EQ(L"Empty id", AsParseError(L"foo#"));
  EXPECT_EQ(L"Empty class", AsParseError(L"foo."));
  EXPECT_EQ(L"Empty pseudo-class", AsParseError(L"foo:"));
  EXPECT_EQ(L"Empty pseudo-class", AsParseError(L"foo::"));

  EXPECT_EQ(L"Empty class", AsParseError(L"."));
  EXPECT_EQ(L"Empty class", AsParseError(L".."));
  EXPECT_EQ(L"Bad class", AsParseError(L".#"));
  EXPECT_EQ(L"Bad class", AsParseError(L".!"));
  EXPECT_EQ(L"Empty class", AsParseError(L".:"));

  EXPECT_EQ(L"Empty class", AsParseError(L".ab."));
  EXPECT_EQ(L"Empty pseudo-class", AsParseError(L".ab:"));
  EXPECT_EQ(L"Bad class", AsParseError(L".ab#"));
}

TEST(CssSelctorTest, ToString) {
  EXPECT_EQ(L"", Selector().ToString());
  EXPECT_EQ(L"foo", Parse(L"foo").ToString());
  EXPECT_EQ(L"#bar", Parse(L"#bar").ToString());
  EXPECT_EQ(L".c1", Parse(L".c1").ToString());
  EXPECT_EQ(L":hover", Parse(L":hover").ToString());
  EXPECT_EQ(L"foo#bar", Parse(L"foo#bar").ToString());
  EXPECT_EQ(L"foo#bar.c1", Parse(L"foo#bar.c1").ToString());
  EXPECT_EQ(L"foo.c1", Parse(L"foo.c1").ToString());
  EXPECT_EQ(L"foo.c1.c2", Parse(L"foo.c1.c2").ToString());
  EXPECT_EQ(L"foo.c1:hover", Parse(L"foo.c1:hover").ToString());
}

}  // namespace css
}  // namespace visuals
