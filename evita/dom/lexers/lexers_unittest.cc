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

TEST_F(LexersTest, JavaLexer) {
  EXPECT_SCRIPT_EQ(
      "c9 w1 c6 w1 k5 w1 i2 w1 o1 w1 s4 o1 w3 k10 w1 i2 w1 o1 w1 s4 o1",
      "applySyntax(JavaLexer, ["
      "'/* foo */',"
      "'// bar',"
      "'class a1 = \\'s1\\';',"
      "'  instanceof a2 = \"s2\";',"
      "])");

  EXPECT_SCRIPT_EQ("w1 k9 w1 k4 i4 w1 k4 o1 k5 o2 w1 i3 o1 k8 o2 w1 i13",
                   "applySyntax(JavaLexer, ['"
                   " @Override"
                   " this.foo"
                   " this.clone()"
                   " Foo.getClass()"
                   " Foo.prototype"
                   "'])");
}

}  // namespace dom
