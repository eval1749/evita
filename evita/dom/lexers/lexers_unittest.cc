// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/abstract_dom_test.h"

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
      "function doColor(fileName, lines) {"
      "  var doc = new Document(fileName);"
      "  var range = new Range(doc);"
      "  range.text = lines.join('\\n');"
      "  doc.mode = Mode.chooseModeByFileName(fileName);"
      "  doc.doColor_(doc.length);"
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
                   "doColor('foo.mk', ["
                   "'# foo',"
                   "'all: object',"
                   "'  echo foo',"
                   "])");
}

TEST_F(LexersTest, CppLexer) {
  EXPECT_SCRIPT_EQ(
      "c9 n1 c6 n1 k8 n1 o1 n3 o1 n1 k4 n4 o1 n1 s4 o1 n3 k4 n4 o1 n1 s4 o1",
      "doColor('foo.cc', ["
      "'/* foo */',"
      "'// bar',"
      "'#include <foo>',"
      "'auto a1 = \\'s1\\';',"
      "'  auto a2 = \"s2\";',"
      "])");

  EXPECT_SCRIPT_EQ(
      "n1 k15 o1 n1 o1 n1 c5 n10 o1 n1 o1 n1 c6 n8",
      "doColor('extended_keywords.cc', ['"
      " std::unique_ptr<T>"
      " std::vector_ptr<T>"  // not extended keyword
      " base::string16"      // chromium specific == not extended keyword
      "'])");
}

TEST_F(LexersTest, IdlLexer) {
  EXPECT_SCRIPT_EQ("c9 n1 c6 n1 k9 n4 o1 n1 s4 o1 n3 k9 n4 o1 n1 s4 o1",
                   "doColor('foo.idl', ["
                   "'/* foo */',"
                   "'// bar',"
                   "'attribute a1 = \\'s1\\';',"
                   "'  interface a2 = \"s2\";',"
                   "])");
}

TEST_F(LexersTest, HtmlLexer) {
  EXPECT_SCRIPT_EQ(
      "k1 e6 k1 n1 k3 n3 o1 n1 s5 o1 n1 k2 e6 k1 n1 k1 e3 k1 n3 k2 e3 k1",
      "doColor('foo.html', ["
      "'<script>',"
      "'var x = \"foo\";',"
      "'</script>',"
      "'<div>foo</div>'"
      "])");
}

TEST_F(LexersTest, JavaLexer) {
  EXPECT_SCRIPT_EQ("c9 n1 c6 n1 k5 n4 o1 n1 s4 o1 n3 k10 n4 o1 n1 s4 o1",
                   "doColor('foo.java', ["
                   "'/* foo */',"
                   "'// bar',"
                   "'class a1 = \\'s1\\';',"
                   "'  instanceof a2 = \"s2\";',"
                   "])");

  EXPECT_SCRIPT_EQ("n11 k4 o1 n4 k4 o1 n5 o2 n4 o1 n8 o2 n4 K10",
                   "doColor('extended_java.js', ['"
                   " @Override"
                   " this.foo"
                   " this.clone()"
                   " Foo.getClass()"
                   " Foo.prototype"
                   "'])");
}

TEST_F(LexersTest, JavaScriptLexer) {
  EXPECT_SCRIPT_EQ("c9 n1 c6 n1 k2 n1 o1 n3 o1 n1 s4 o1 n3 k3 n4 o1 n1 s4 o1",
                   "doColor('foo.js', ["
                   "'/* foo */',"
                   "'// bar',"
                   "'if (a1 = \\'s1\\')',"
                   "'  var a2 = \"s2\";',"
                   "])");

  EXPECT_SCRIPT_EQ("n1 k8 o1 n1 o1 n1 k4 o1 n4 k4 K7 n4 K7 n4 K10 n4 K9 o2",
                   "doColor('extended_keywords.js', ['"
                   " Math.sin(1)"
                   " this.foo"
                   " this.length"
                   " Foo.length"
                   " Foo.prototype"
                   " Foo.toString()"
                   "'])");
}

TEST_F(LexersTest, PythonLexer) {
  EXPECT_SCRIPT_EQ("c5 n1 c5 n1 k2 n4 o2 n1 s4 o1 n8 o1 n1 s4",
                   "doColor('foo.py', ["
                   "'# bar',"
                   "'# foo',"
                   "'if a1 == \\'s1\\':',"
                   "'    a2 = \"s2\"',"
                   "])");
}

TEST_F(LexersTest, XmlLexer) {
  EXPECT_SCRIPT_EQ(
      "c16 n1 k1 e7 n1 a5 n2 v3 n2 a5 n2 v3 n1 k1 n4 &5 n4 k2 e7 k1",
      "doColor('foo.xml', ["
      "'<!-- comment -->',"
      "'<element attr1=\\'123\\' attr2=\"456\">',"
      "'foo&amp;bar',"
      "'</element>',"
      "])");

  EXPECT_SCRIPT_EQ("k1 e2 k2", "doColor('foo.xml', ['<br/>'])");

  EXPECT_SCRIPT_EQ("k2 e3 n1 a7 n2 v3 n1 k2",
                   "doColor('foo.xml', ['<?xml version=\"1.0\"?>'])");

  // attribute variations
  //  <input autofocus maxlength=10 />
  EXPECT_SCRIPT_EQ("k1 e5 n1 a3 n1 a3 n1 v2 k1",
                   "doColor('foo.xml', ['<input abc xyz=10>'])");

  // Ampersand
  EXPECT_SCRIPT_EQ("n5 &5 n1 &6 n1 &8 n4",
                   "doColor('foo.html', ['& && &amp; &#123; &#xBEEF; foo'])");

  // Malformed:  no space between attributes.
  //  <abc def="123"ghi>
  EXPECT_SCRIPT_EQ("k1 e3 n1 a3 n2 v3 n1 a3 k1",
                   "doColor('foo.html', ['<abc def=\"123\"ghi>'])");

  // Malformed: no ending quote
  //  <abc def="123>ghi
  EXPECT_SCRIPT_EQ("k1 e3 n1 a3 n2 v3 k1 n3",
                   "doColor('foo.html', ['<abc def=\"123>ghi'])");
}

}  // namespace dom
