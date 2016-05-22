// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <set>
#include <sstream>
#include <vector>

#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"
#include "evita/css/selector_builder.h"
#include "evita/css/selector_parser.h"
#include "testing/gtest/include/gtest/gtest.h"

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

Selector Parse(base::StringPiece text) {
  return std::move(Selector::Parser().Parse(base::UTF8ToUTF16(text)));
}

}  // namespace

TEST(CssSelctorTest, Equals) {
  Selector selector0;
  const auto& selector1 = Parse("foo");
  const auto& selector11 = Parse("foo");
  const auto& selector2 = Parse("bar");
  const auto& selector22 = Parse("bar");
  const auto& selector3 = Parse("foo.c1");
  const auto& selector33 = Parse("foo.c1");
  const auto& selector4 = Parse("foo#bar.c1");
  const auto& selector44 = Parse("foo#bar.c1");
  const auto& selector5 = Parse("foo#bar.c1.c2");
  const auto& selector55 = Parse("foo#bar.c1.c2");

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
  EXPECT_EQ(Parse("*"), MoreSpecific(Parse("*"), Parse("*")));
  EXPECT_EQ(Parse("foo"), MoreSpecific(Parse("foo"), Parse("*")));
  EXPECT_EQ(Parse("foo"), MoreSpecific(Parse("foo"), Parse("foo")));
  EXPECT_EQ(Parse("#bar"), MoreSpecific(Parse("foo"), Parse("#bar")));
  EXPECT_EQ(Parse("foo#bar"), MoreSpecific(Parse("foo#bar"), Parse("#bar")));
  EXPECT_EQ(Parse("foo#bar"), MoreSpecific(Parse("foo#bar"), Parse("foo.c1")));
  EXPECT_EQ(Parse("foo.c1"), MoreSpecific(Parse("foo.c1"), Parse("foo")));
  EXPECT_EQ(Parse("foo.c2"), MoreSpecific(Parse("foo.c1"), Parse("foo.c2")));
  EXPECT_EQ(Parse("foo.c1.c2"),
            MoreSpecific(Parse("foo.c1.c2"), Parse("foo.c2")));

  EXPECT_TRUE(Parse(".c1").IsMoreSpecific(Parse("tag")))
      << "Class selector is more specific than type selector.";
  EXPECT_FALSE(Parse("tag").IsMoreSpecific(Parse(".c1")))
      << "Class selector is more specific than type selector.";

  EXPECT_FALSE(Parse(".c1").IsMoreSpecific(Parse(".c2")))
      << "We can't determine specificity for '.c1' and '.c2'.";
  EXPECT_FALSE(Parse(".c2").IsMoreSpecific(Parse(".c1")))
      << "We can't determine specificity for '.c1' and '.c2'.";
}

TEST(CssSelctorTest, IsSubsetOf) {
  EXPECT_TRUE(Parse("*").IsSubsetOf(Parse("*")));
  EXPECT_FALSE(Parse("*").IsSubsetOf(Parse("*#bar")));
  EXPECT_FALSE(Parse("*").IsSubsetOf(Parse("*.c1")));
  EXPECT_FALSE(Parse("*").IsSubsetOf(Parse("foo")));
  EXPECT_FALSE(Parse("*").IsSubsetOf(Parse("foo#bar")));
  EXPECT_FALSE(Parse("*").IsSubsetOf(Parse("foo.c1")));
  EXPECT_FALSE(Parse("*").IsSubsetOf(Parse("foo.c1.c2")));
  EXPECT_FALSE(Parse("*").IsSubsetOf(Parse("foo:hover")));

  EXPECT_TRUE(Parse("foo").IsSubsetOf(Parse("*")));
  EXPECT_FALSE(Parse("foo").IsSubsetOf(Parse("*#bar")));
  EXPECT_FALSE(Parse("foo").IsSubsetOf(Parse("*.c1")));
  EXPECT_TRUE(Parse("foo").IsSubsetOf(Parse("foo")));
  EXPECT_FALSE(Parse("foo").IsSubsetOf(Parse("foo#bar")));
  EXPECT_FALSE(Parse("foo").IsSubsetOf(Parse("foo.c1")));
  EXPECT_FALSE(Parse("foo").IsSubsetOf(Parse("foo:hover")));

  EXPECT_TRUE(Parse("foo#bar").IsSubsetOf(Parse("*")));
  EXPECT_TRUE(Parse("foo#bar").IsSubsetOf(Parse("*#bar")));
  EXPECT_FALSE(Parse("foo#bar").IsSubsetOf(Parse("*.c1")));
  EXPECT_TRUE(Parse("foo#bar").IsSubsetOf(Parse("foo")));
  EXPECT_TRUE(Parse("foo#bar").IsSubsetOf(Parse("foo#bar")));
  EXPECT_FALSE(Parse("foo#bar").IsSubsetOf(Parse("foo.c1")));
  EXPECT_FALSE(Parse("foo#bar").IsSubsetOf(Parse("foo.c1.c2")));
  EXPECT_FALSE(Parse("foo#bar").IsSubsetOf(Parse("foo:hover")));

  EXPECT_TRUE(Parse("foo.c1").IsSubsetOf(Parse("*")));
  EXPECT_FALSE(Parse("foo.c1").IsSubsetOf(Parse("*#bar")));
  EXPECT_TRUE(Parse("foo.c1").IsSubsetOf(Parse("*.c1")));
  EXPECT_TRUE(Parse("foo.c1").IsSubsetOf(Parse("foo")));
  EXPECT_FALSE(Parse("foo.c1").IsSubsetOf(Parse("foo#bar")));
  EXPECT_TRUE(Parse("foo.c1").IsSubsetOf(Parse("foo.c1")));
  EXPECT_FALSE(Parse("foo.c1").IsSubsetOf(Parse("foo.c1.c2")));
  EXPECT_FALSE(Parse("foo.c1").IsSubsetOf(Parse("foo:hover")));

  EXPECT_TRUE(Parse("foo.c1.c2").IsSubsetOf(Parse("*")));
  EXPECT_FALSE(Parse("foo.c1.c2").IsSubsetOf(Parse("*#bar")));
  EXPECT_TRUE(Parse("foo.c1.c2").IsSubsetOf(Parse("*.c1")));
  EXPECT_TRUE(Parse("foo.c1.c2").IsSubsetOf(Parse("foo")));
  EXPECT_FALSE(Parse("foo.c1.c2").IsSubsetOf(Parse("foo#bar")));
  EXPECT_TRUE(Parse("foo.c1.c2").IsSubsetOf(Parse("foo.c1")));
  EXPECT_TRUE(Parse("foo.c1.c2").IsSubsetOf(Parse("foo.c1.c2")));
  EXPECT_FALSE(Parse("foo.c1.c2").IsSubsetOf(Parse("foo:hover")));
}

TEST(CssSelctorTest, Less) {
  std::set<Selector> set;
  set.insert(Parse(""));
  set.insert(Parse("foo"));
  set.insert(Parse("bar"));
  set.insert(Parse("foo#id1"));
  set.insert(Parse("foo#id1.c1"));
  set.insert(Parse("foo#id1.c1.c2"));
  set.insert(Parse("foo.c2"));
  set.insert(Parse("foo.c2.c3"));

  EXPECT_EQ((std::vector<css::Selector>{
                Parse(""), Parse("foo.c2.c3"), Parse("foo.c2"), Parse("foo"),
                Parse("foo#id1.c1.c2"), Parse("foo#id1.c1"), Parse("foo#id1"),
                Parse("bar")}),
            std::vector<Selector>(set.begin(), set.end()));

  EXPECT_FALSE(Parse("zoo") < Parse(".c1"));
  EXPECT_FALSE(Parse("zoo") < Parse(":hover"));
  EXPECT_TRUE(Parse(":hover.c1") < Parse(".c1"));
  EXPECT_FALSE(Parse(".c1") < Parse(":hover.c1"));

  EXPECT_FALSE(Parse(".c1.c2") < Parse(".c1.c2"));
  EXPECT_TRUE(Parse(".c1.c2") < Parse(".c1"));
  EXPECT_TRUE(Parse(".c1.c2") < Parse(".c2"));
}

TEST(CssSelctorTest, Parser) {
  EXPECT_EQ(Selector(), Parse(""));
  EXPECT_EQ(Selector(), Parse("*"));
  EXPECT_EQ(AsSelector(L"", L"bar", {}), Parse("*#bar"));
  EXPECT_EQ(AsSelector(L"", {L"c1"}), Parse("*.c1"));
  EXPECT_EQ(AsSelector(L"", {L":hover"}), Parse("*:hover"));
  EXPECT_EQ(AsSelector(L"foo"), Parse("foo"));
  EXPECT_EQ(AsSelector(L"foo", L"bar", {}), Parse("foo#bar"));
  EXPECT_EQ(AsSelector(L"foo", {L"a"}), Parse("foo.a"));
  EXPECT_EQ(AsSelector(L"foo", {L":hover"}), Parse("foo:hover"));
  EXPECT_EQ(AsSelector(L"foo", L"bar", {L"c1"}), Parse("foo#bar.c1"));
  EXPECT_EQ(AsSelector(L"foo", L"bar", {L"c1", L"c2"}), Parse("foo#bar.c1.c2"));
  EXPECT_EQ(AsSelector(L"::selector"), Parse("::selector"));
  EXPECT_EQ(AsSelector(L"::selector", {L":active"}),
            Parse("::selector:active"));
  EXPECT_EQ(AsSelector(L"a9"), Parse("a9"));
  EXPECT_EQ(AsSelector(L"Zero"), Parse("Zero"));
  EXPECT_EQ(AsSelector(L"zoo"), Parse("zoo"));
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

TEST(CssSelctorTest, is_universal) {
  EXPECT_TRUE(Parse("*").is_universal());
  EXPECT_TRUE(Parse("*#bar").is_universal());
  EXPECT_TRUE(Parse("*.c1").is_universal());
  EXPECT_TRUE(Parse("*.c1.c2").is_universal());
  EXPECT_TRUE(Parse("*:hover").is_universal());
  EXPECT_TRUE(Parse("#bar").is_universal());
  EXPECT_TRUE(Parse(".c1").is_universal());
  EXPECT_TRUE(Parse(".c1.c2").is_universal());
  EXPECT_TRUE(Parse(":hover").is_universal());
  EXPECT_FALSE(Parse("foo").is_universal());
  EXPECT_FALSE(Parse("foo#bar").is_universal());
  EXPECT_FALSE(Parse("foo.c1").is_universal());
  EXPECT_FALSE(Parse("foo.c1.c2").is_universal());
  EXPECT_FALSE(Parse("foo:hover").is_universal());
}

TEST(CssSelctorTest, ToString) {
  EXPECT_EQ(L"*", Selector().ToString());
  EXPECT_EQ(L"foo", Parse("foo").ToString());
  EXPECT_EQ(L"#bar", Parse("#bar").ToString());
  EXPECT_EQ(L".c1", Parse(".c1").ToString());
  EXPECT_EQ(L":hover", Parse(":hover").ToString());
  EXPECT_EQ(L"foo#bar", Parse("foo#bar").ToString());
  EXPECT_EQ(L"foo#bar.c1", Parse("foo#bar.c1").ToString());
  EXPECT_EQ(L"foo.c1", Parse("foo.c1").ToString());
  EXPECT_EQ(L"foo.c1.c2", Parse("foo.c1.c2").ToString());
  EXPECT_EQ(L"foo.c1:hover", Parse("foo.c1:hover").ToString());
}

}  // namespace css
