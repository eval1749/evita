// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/abstract_dom_test.h"

namespace {

class LexersTest : public dom::AbstractDomTest {
  public: LexersTest() = default;
  public: ~LexersTest() = default;

  DISALLOW_COPY_AND_ASSIGN(LexersTest);
};

TEST_F(LexersTest, all) {
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
    "}"
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

  // ConfigLexer
  EXPECT_SCRIPT_EQ(
    "c5 n23",
    "doColor('foo.mk', ["
    "'# foo',"
    "'all: object',"
    "'  echo foo',"
    "])");

  // CppLexer
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
    "n1 k15 o1 n1 o1 n4 o2 n10 o1 n1 o1 n1 k14",
    "doColor('extended_keywords.cc', ['"
        " std::unique_ptr<T>"
        " std::vector_ptr<T>"
        " base::string16"
    "'])");

  // IdlLexer
  EXPECT_SCRIPT_EQ(
    "c9 n1 c6 n1 k9 n4 o1 n1 s4 o1 n3 k9 n4 o1 n1 s4 o1",
    "doColor('foo.idl', ["
        "'/* foo */',"
        "'// bar',"
        "'attribute a1 = \\'s1\\';',"
        "'  interface a2 = \"s2\";',"
    "])");

  // JavaLexer
  EXPECT_SCRIPT_EQ(
    "c9 n1 c6 n1 k5 n4 o1 n1 s4 o1 n3 k10 n4 o1 n1 s4 o1",
    "doColor('foo.java', ["
        "'/* foo */',"
        "'// bar',"
        "'class a1 = \\'s1\\';',"
        "'  instanceof a2 = \"s2\";',"
    "])");

  EXPECT_SCRIPT_EQ(
    "n11 k4 o1 n4 k4 o1 n5 o2 n4 o1 n8 o2 n4 K10",
    "doColor('extended_java.js', ['"
        " @Override"
        " this.foo"
        " this.clone()"
        " Foo.getClass()"
        " Foo.prototype"
    "'])");

  // JavaScriptLexer
  EXPECT_SCRIPT_EQ(
    "c9 n1 c6 n1 k2 n1 o1 n3 o1 n1 s4 o1 n3 k3 n4 o1 n1 s4 o1",
    "doColor('foo.js', ["
        "'/* foo */',"
        "'// bar',"
        "'if (a1 = \\'s1\\')',"
        "'  var a2 = \"s2\";',"
    "])");

  EXPECT_SCRIPT_EQ(
    "n1 k8 o1 n1 o1 n1 k4 o1 n4 k4 K7 n4 K7 n4 K10 n4 K9 o2",
    "doColor('extended_keywords.js', ['"
        " Math.sin(1)"
        " this.foo"
        " this.length"
        " Foo.length"
        " Foo.prototype"
        " Foo.toString()"
    "'])");

  // PythonLexer
  EXPECT_SCRIPT_EQ(
    "c5 n1 c5 n1 k2 n4 o2 n1 s4 o1 n8 o1 n1 s4",
    "doColor('foo.py', ["
        "'# bar',"
        "'# foo',"
        "'if a1 == \\'s1\\':',"
        "'    a2 = \"s2\"',"
    "])");

  // XmlLexer
  EXPECT_SCRIPT_EQ(
    "c16 n2 e7 n1 a5 n1 v5 n1 a5 n1 v5 n5 &5 n6 e7 n1",
    "doColor('foo.xml', ["
        "'<!-- comment -->',"
        "'<element attr1=\\'123\\' attr2=\"456\">',"
        "'foo&amp;bar',"
        "'</element>',"
    "])");

  // attribute variations
  //  <input autofocus maxlength=10 />
  EXPECT_SCRIPT_EQ(
    "n1 e5 n1 a3 n1 a3 n1 v2 n1",
    "doColor('foo.xml', ['<input abc xyz=10>'])");

  // Ampersand
  EXPECT_SCRIPT_EQ(
    "n5 &5 n1 &6 n1 &8 n4",
    "doColor('foo.html', ['& && &amp; &#123; &#xBEEF; foo'])");

  // Malformed:  no space between attributes.
  //  <abc def="123"ghi>
  EXPECT_SCRIPT_EQ(
    "n1 e3 n1 a3 n1 v5 a3 n1",
    "doColor('foo.html', ['<abc def=\"123\"ghi>'])");

  // Malformed: no ending quote
  //  <abc def="123>ghi
  EXPECT_SCRIPT_EQ(
    "n1 e3 n1 a3 n1 v4 n4",
    "doColor('foo.html', ['<abc def=\"123>ghi'])");
}

}  // namespace
