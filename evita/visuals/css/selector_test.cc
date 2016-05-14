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
  return parser.error().as_string();
}

std::string AsString(const std::vector<Selector>& selectors) {
  std::ostringstream ostream;
  auto delimiter = "{";
  for (const auto& selector : selectors) {
    ostream << delimiter << selector;
    delimiter = ", ";
  }
  ostream << '}';
  return ostream.str();
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
  return builder.Build();
}

Selector AsSelector(base::StringPiece16 tag_name,
                    const std::vector<base::StringPiece16>& class_names) {
  return AsSelector(tag_name, L"", class_names);
}

Selector AsSelector(base::StringPiece16 tag_name) {
  return AsSelector(tag_name, L"", {});
}

std::string AsString(const Selector& selector) {
  std::ostringstream ostream;
  ostream << selector;
  return ostream.str();
}
}  // namespace

TEST(CssSelctorTest, Equals) {
  Selector selector0;
  const auto& selector1 = Selector::Builder().SetTagName(L"foo").Build();
  const auto& selector11 = Selector::Builder().SetTagName(L"foo").Build();
  const auto& selector2 = Selector::Builder().SetTagName(L"bar").Build();
  const auto& selector22 = Selector::Builder().SetTagName(L"bar").Build();
  const auto& selector3 =
      Selector::Builder().SetTagName(L"foo").SetId(L"1").Build();
  const auto& selector33 =
      Selector::Builder().SetTagName(L"foo").SetId(L"1").Build();
  const auto& selector4 =
      Selector::Builder().SetTagName(L"foo").SetId(L"1").AddClass(L"a").Build();
  const auto& selector44 =
      Selector::Builder().SetTagName(L"foo").SetId(L"1").AddClass(L"a").Build();
  const auto& selector5 = Selector::Builder()
                              .SetTagName(L"foo")
                              .SetId(L"1")
                              .AddClass(L"a")
                              .AddClass(L"b")
                              .Build();
  const auto& selector55 = Selector::Builder()
                               .SetTagName(L"foo")
                               .SetId(L"1")
                               .AddClass(L"a")
                               .AddClass(L"b")
                               .Build();

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

TEST(CssSelctorTest, Less) {
  Selector selector0;
  const auto& selector1 = Selector::Builder().SetTagName(L"foo").Build();
  const auto& selector2 = Selector::Builder().SetTagName(L"bar").Build();
  const auto& selector3 =
      Selector::Builder().SetTagName(L"foo").SetId(L"1").Build();
  const auto& selector4 =
      Selector::Builder().SetTagName(L"foo").SetId(L"1").AddClass(L"a").Build();
  const auto& selector5 = Selector::Builder()
                              .SetTagName(L"foo")
                              .SetId(L"1")
                              .AddClass(L"a")
                              .AddClass(L"b")
                              .Build();

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
  EXPECT_EQ(Selector(), Selector::Parser().Parse(L""));
  EXPECT_EQ(AsSelector(L"foo"), Selector::Parser().Parse(L"foo"));
  EXPECT_EQ(AsSelector(L"foo", L"bar", {}),
            Selector::Parser().Parse(L"foo#bar"));
  EXPECT_EQ(AsSelector(L"foo", {L"a"}), Selector::Parser().Parse(L"foo.a"));
  EXPECT_EQ(AsSelector(L"foo", {L":hover"}),
            Selector::Parser().Parse(L"foo:hover"));
  EXPECT_EQ(AsSelector(L"foo", L"bar", {L"abc"}),
            Selector::Parser().Parse(L"foo#bar.abc"));
  EXPECT_EQ(AsSelector(L"foo", L"bar", {L"abc", L"def"}),
            Selector::Parser().Parse(L"foo#bar.abc.def"));
  EXPECT_EQ(AsSelector(L"::selector"), Selector::Parser().Parse(L"::selector"));
  EXPECT_EQ(AsSelector(L"::selector", {L":active"}),
            Selector::Parser().Parse(L"::selector:active"));
}

TEST(CssSelctorTest, ParseError) {
  EXPECT_EQ(L"Bad tag name", AsParseError(L"!"));

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

TEST(CssSelctorTest, Printer) {
  EXPECT_EQ("", AsString(Selector()));
  EXPECT_EQ("foo", AsString(Selector::Builder().SetTagName(L"foo").Build()));
  EXPECT_EQ("#bar", AsString(Selector::Builder().SetId(L"bar").Build()));
  EXPECT_EQ(".a", AsString(Selector::Builder().AddClass(L"a").Build()));
  EXPECT_EQ(":hover",
            AsString(Selector::Builder().AddClass(L":hover").Build()));
  EXPECT_EQ(
      "foo#bar",
      AsString(Selector::Builder().SetTagName(L"foo").SetId(L"bar").Build()));
  EXPECT_EQ("foo#bar.a", AsString(Selector::Builder()
                                      .SetTagName(L"foo")
                                      .SetId(L"bar")
                                      .AddClass(L"a")
                                      .Build()));
  EXPECT_EQ(
      "foo.a",
      AsString(Selector::Builder().SetTagName(L"foo").AddClass(L"a").Build()));
  EXPECT_EQ("foo.a.b", AsString(Selector::Builder()
                                    .SetTagName(L"foo")
                                    .AddClass(L"b")
                                    .AddClass(L"a")
                                    .Build()));
  EXPECT_EQ("foo.a:hover", AsString(Selector::Builder()
                                        .SetTagName(L"foo")
                                        .AddClass(L":hover")
                                        .AddClass(L"a")
                                        .Build()));
}

}  // namespace css
}  // namespace visuals
