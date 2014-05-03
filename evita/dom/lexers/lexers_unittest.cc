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
    "    var syntax = doc.syntaxAt(i).charAt(0);"
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

  // JavaScriptLexer
  EXPECT_SCRIPT_EQ(
    "c9 n1 c6 n1 k2 n1 o1 n3 o1 n1 s4 o1 n3 k3 n4 o1 n1 s4 o1",
    "doColor('foo.js', ["
        "'/* foo */',"
        "'// bar',"
        "'if (a1 = \\'s1\\')',"
        "'  var a2 = \"s2\";',"
    "])");

  // PythonLexer
  EXPECT_SCRIPT_EQ(
    "c5 n1 c5 n1 k2 n4 o2 n1 s4 o1 n8 o1 n1 s4",
    "doColor('foo.py', ["
        "'# bar',"
        "'# foo',"
        "'if a1 == \\'s1\\':',"
        "'    a2 = \"s2\"',"
    "])");
}

}  // namespace
