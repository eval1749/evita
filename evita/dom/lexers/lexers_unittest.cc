// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/testing/abstract_dom_test.h"

namespace dom {

class LexersTest : public AbstractDomTest {
 public:
  ~LexersTest() override = default;

 protected:
  LexersTest() = default;

 private:
  // ::testing::Test
  void SetUp() override;

  DISALLOW_COPY_AND_ASSIGN(LexersTest);
};

void LexersTest::SetUp() {
  AbstractDomTest::SetUp();

  EXPECT_SCRIPT_VALID(
      "function applySyntax(lexerClass, lines) {"
      "  var doc = TextDocument.new(lexerClass.name);"
      "  var range = new TextRange(doc);"
      "  range.text = lines.join('\\n');"
      "  (new lexerClass(doc)).doColor(doc.length);"
      "  var result = '';"
      "  var lastSyntax = '';"
      "  var count = 0;"
      "  for (var i = 0; i < doc.length; ++i) {"
      "    var syntax = symbolize(doc.syntaxAt(i));"
      "    if (count && syntax != lastSyntax) {"
      "      result += lastSyntax + count + ' ';"
      "      count = 0;"
      "    }"
      "    lastSyntax = syntax;"
      "    ++count;"
      "  }"
      "  result += lastSyntax + count;"
      "  return result;"
      "}");

  EXPECT_SCRIPT_VALID(
      "function symbolize(syntax) {"
      "  if (syntax == 'html_attribute_value')"
      "   return 'v';"
      "  if (syntax == 'html_entity')"
      "   return '&';"
      "  if (syntax == 'keyword2')"
      "   return 'K';"
      "  if (syntax.startsWith('html_'))"
      "   return syntax.charAt(5);"
      "  return syntax.charAt(0);"
      "}");
}

TEST_F(LexersTest, ConfigLexer) {
  EXPECT_SCRIPT_EQ("c5 n23",
                   "applySyntax(ConfigLexer, ["
                   "'# foo',"
                   "'all: object',"
                   "'  echo foo',"
                   "])");
}

TEST_F(LexersTest, CppLexer) {
  EXPECT_SCRIPT_EQ(
      "c9 w1 c6 w1 k8 w1 o1 i3 o1 w1 k4 w1 i2 w1 o1 w1 s4 o1 w3 k4 w1 i2 w1"
      " o1 w1 s4 o1",
      "applySyntax(CppLexer, ["
      "'/* foo */',"
      "'// bar',"
      "'#include <foo>',"
      "'auto a1 = \\'s1\\';',"
      "'  auto a2 = \"s2\";',"
      "])");

  EXPECT_SCRIPT_EQ(
      "w1 k15 o1 i1 o1 w1 i15 o1 i1 o1 w1 i14",
      "applySyntax(CppLexer, ['"
      " std::unique_ptr<T>"
      " std::vector_ptr<T>"  // not extended keyword
      " base::string16"      // chromium specific == not extended keyword
      "'])");
}

TEST_F(LexersTest, IdlLexer) {
  EXPECT_SCRIPT_EQ("c9 n1 c6 n1 k9 n4 o1 n1 s4 o1 n3 k9 n4 o1 n1 s4 o1",
                   "applySyntax(IdlLexer, ["
                   "'/* foo */',"
                   "'// bar',"
                   "'attribute a1 = \\'s1\\';',"
                   "'  interface a2 = \"s2\";',"
                   "])");
}

TEST_F(LexersTest, HtmlLexer) {
  EXPECT_SCRIPT_EQ("k1 e6 k1 a25 n1 k1 e3 k1 n3 k2 e3 k1",
                   "applySyntax(HtmlLexer, ["
                   "'<script>',"
                   "'var x = \"foo\";',"
                   "'</script>',"
                   "'<div>foo</div>'"
                   "])");
}

TEST_F(LexersTest, JavaLexer) {
  EXPECT_SCRIPT_EQ("c9 n1 c6 n1 k5 n4 o1 n1 s4 o1 n3 k10 n4 o1 n1 s4 o1",
                   "applySyntax(JavaLexer, ["
                   "'/* foo */',"
                   "'// bar',"
                   "'class a1 = \\'s1\\';',"
                   "'  instanceof a2 = \"s2\";',"
                   "])");

  EXPECT_SCRIPT_EQ("n1 k9 n1 k4 o1 n4 k10 o2 n4 k9 o2 n4 o1 n9",
                   "applySyntax(JavaLexer, ['"
                   " @Override"
                   " this.foo"
                   " this.clone()"
                   " Foo.getClass()"
                   " Foo.prototype"
                   "'])");
}

TEST_F(LexersTest, JavaScriptLexer) {
  EXPECT_SCRIPT_EQ("c9 n1 c6 n1 k2 n1 o1 n3 o1 n1 s4 o1 n3 k3 n4 o1 n1 s4 o1",
                   "applySyntax(JavaScriptLexer, ["
                   "'/* foo */',"
                   "'// bar',"
                   "'if (a1 = \\'s1\\')',"
                   "'  var a2 = \"s2\";',"
                   "])");

  EXPECT_SCRIPT_EQ("n1 k8 o1 n1 o1 n1 k4 o1 n4 k4 K7 n4 K7 n4 K10 n4 K9 o2",
                   "applySyntax(JavaScriptLexer, ['"
                   " Math.sin(1)"
                   " this.foo"
                   " this.length"
                   " Foo.length"
                   " Foo.prototype"
                   " Foo.toString()"
                   "'])");
}

}  // namespace dom
