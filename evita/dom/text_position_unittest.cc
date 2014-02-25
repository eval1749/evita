// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/basictypes.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/text/style.h"

namespace {

class TextPositionTest : public dom::AbstractDomTest {
  protected: TextPositionTest() {
    g_DefaultStyle.m_rgfMask = StyleValues::Mask_Background |
                               StyleValues::Mask_Color |
                               StyleValues::Mask_Decoration |
                               StyleValues::Mask_FontFamily |
                               StyleValues::Mask_FontStyle |
                               StyleValues::Mask_FontWeight |
                               StyleValues::Mask_Marker |
                               StyleValues::Mask_Syntax;
    g_DefaultStyle.m_crBackground = Color(255, 255, 255);
    g_DefaultStyle.m_crColor = Color(0x00, 0x00, 0x00);
    g_DefaultStyle.m_crMarker = Color(0x00, 0x99, 0x00);
  }
  public: virtual ~TextPositionTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(TextPositionTest);
};

TEST_F(TextPositionTest, Basic) {
  EXPECT_SCRIPT_VALID(
    "var doc = new Document('style');"
    "var range = new Range(doc);"
    "range.text = 'foo bar baz';"
    "var pos = new TextPosition(doc, 2);");
  EXPECT_SCRIPT_TRUE("pos.document == doc");
  EXPECT_SCRIPT_EQ("2", "pos.offset");
  EXPECT_SCRIPT_EQ("111", "pos.charCode()");
  EXPECT_SCRIPT_EQ("0", "pos.charSyntax()");

  EXPECT_SCRIPT_EQ("RangeError: Invalid offset -1 for [object Document]",
                   "new TextPosition(doc, -1)");
  EXPECT_SCRIPT_EQ("RangeError: Invalid offset 100 for [object Document]",
                   "new TextPosition(doc, 100)");
}

TEST_F(TextPositionTest, move) {
  EXPECT_SCRIPT_VALID(
    "var doc = new Document('style');"
    "var range = new Range(doc);"
    "range.text = 'foo bar baz';"
    "var pos = new TextPosition(doc, 0);");
  EXPECT_SCRIPT_EQ("1", "pos.move(Unit.CHARACTER).offset");
  EXPECT_SCRIPT_EQ("3", "pos.move(Unit.CHARACTER, 2).offset");
  EXPECT_SCRIPT_EQ("2", "pos.move(Unit.CHARACTER, -1).offset");
}

TEST_F(TextPositionTest, moveBracket) {
  EXPECT_SCRIPT_VALID(
    "var doc = new Document('moveBracket');"
    "var range = new Range(doc);"
    "function testIt(sample, direction) {"
    "  range.start = 0;"
    "  range.end = doc.length;"
    "  var text = sample.replace('^', '').replace(/[|]/g, '\"')"
    "       .replace(/[#]/g, '\\\\');"
    "  range.text = text;"
    "  highlight(doc);"
    "  var pos = new TextPosition(doc, sample.indexOf('^'));"
    "  pos.move(Unit.BRACKET, direction);"
    "  var text2 = text.replace(/\"/g, '|').replace(/[\\\\]/g, '#');"
    "  return text2.substr(0, pos.offset) + '^' + text2.substr(pos.offset);"
    "}"
    "function backward(sample) { return testIt(sample, -1); }"
    "function forward(sample) { return testIt(sample, 1); }"
    "function highlight(document) {"
    "  var pos = new TextPosition(document, 0);"
    "  var range = new Range(document);"
    "  var state = 'NORMAL';"
    "  var color = 0;"
    "  var char_syntax = 0;"
    "  function finish(advance) {"
    "    range.end = pos.offset + advance;"
    "    range.style({charSyntax: char_syntax, color: color});"
    "    range.collapseTo(range.end);"
    "    state = 'NORMAL';"
    "    color = 0;"
    "    char_syntax = 0;"
    "  }"
    "  for (; pos.offset < document.length; pos.move(Unit.CHARACTER)) {"
    "    var char_code = pos.charCode();"
    "    switch (state) {"
    "      case 'NORMAL':"
    "        if (char_code == Unicode.QUOTATION_MARK) {"
    "          finish(0);"
    "          state = 'STRING1';"
    "          color = 0x000080;"
    "          char_syntax = 1;"
    "        } else if (char_code == Unicode.SOLIDUS) {"
    "          finish(0);"
    "          state = 'COMMENT_1';"
    "          color = 0x008000;"
    "          char_syntax = 2;"
    "        } else if (char_code == Unicode.LEFT_CURLY_BRACKET ||"
    "                   char_code == Unicode.RIGHT_CURLY_BRACKET) {"
    "          finish(0);"
    "          color = 0;"
    "          char_syntax = 3;"
    "          finish(1);"
    "          pos.move(Unit.CHARACTER);"
    "        }"
    "        break;"
    "      case 'COMMENT_1':"
    "        if (char_code == Unicode.SOLIDUS)"
    "          finish(1);"
    "        break;"
    "      case 'STRING1':"
    "        if (char_code == Unicode.QUOTATION_MARK)"
    "          finish(1);"
    "        else if (char_code == Unicode.REVERSE_SOLIDUS)"
    "          state = 'STRING1_ESCAPE';"
    "        break;"
    "      case 'STRING1_ESCAPE':"
    "        state = 'STRING1';"
    "        break;"
    "    }"
    "  }"
    "}");

  // forward
  EXPECT_SCRIPT_EQ("(foo)^ (bar)", "forward('^(foo) (bar)')");
  EXPECT_SCRIPT_EQ("(foo)^ (bar)", "forward('(^foo) (bar)')");
  EXPECT_SCRIPT_EQ("(foo)^ (bar)", "forward('(f^oo) (bar)')");
  EXPECT_SCRIPT_EQ("(foo)^ (bar)", "forward('(fo^o) (bar)')");
  EXPECT_SCRIPT_EQ("(foo)^ (bar)", "forward('(foo^) (bar)')");
  EXPECT_SCRIPT_EQ("(foo) ^(bar)", "forward('(foo)^ (bar)')");
  EXPECT_SCRIPT_EQ("(foo) (bar)^", "forward('(foo) ^(bar)')");

  // bracket and others have different colors.
  EXPECT_SCRIPT_EQ("} else ^{ foo", "forward('}^ else { foo')");

  // string is a different matching context.
  EXPECT_SCRIPT_EQ("(foo |bar)| baz)^", "forward('^(foo |bar)| baz)')");
  EXPECT_SCRIPT_EQ("|(foo| bar) |baz)^|", "forward('|^(foo| bar) |baz)|')");

  // escape
  EXPECT_SCRIPT_EQ("(foo #) bar)^ baz", "forward('^(foo #) bar) baz')");
  EXPECT_SCRIPT_EQ("(foo ##)^ bar) baz", "forward('^(foo ##) bar) baz')");

  // mismatched
  EXPECT_SCRIPT_EQ("(foo^] bar", "forward('^(foo] bar')");

  // nested parenthesis
  EXPECT_SCRIPT_EQ("((foo))^", "forward('^((foo))')");
  EXPECT_SCRIPT_EQ("((foo)^)", "forward('(^(foo))')");

  // backward
  EXPECT_SCRIPT_EQ("^(foo) (bar)", "backward('^(foo) (bar)')");
  EXPECT_SCRIPT_EQ("^(foo) (bar)", "backward('(^foo) (bar)')");
  EXPECT_SCRIPT_EQ("^(foo) (bar)", "backward('(f^oo) (bar)')");
  EXPECT_SCRIPT_EQ("^(foo) (bar)", "backward('(fo^o) (bar)')");
  EXPECT_SCRIPT_EQ("^(foo) (bar)", "backward('(foo^) (bar)')");
  EXPECT_SCRIPT_EQ("^(foo) (bar)", "backward('(foo)^ (bar)')");
  EXPECT_SCRIPT_EQ("(foo)^ (bar)", "backward('(foo) ^(bar)')");

  // bracket and others have different colors.
  EXPECT_SCRIPT_EQ("}^ else { foo", "backward('} else ^{ foo')");

  // string is a different matching context.
  EXPECT_SCRIPT_EQ("^(foo |(bar| baz)", "backward('(foo |(bar| baz)^')");

  // escape
  EXPECT_SCRIPT_EQ("^(foo #( bar) baz", "backward('(foo #( bar)^ baz')");
  EXPECT_SCRIPT_EQ("(foo ##^( bar) baz", "backward('(foo ##( bar)^ baz')");

  // mismatched
  EXPECT_SCRIPT_EQ("(^foo] bar", "backward('(foo]^ bar')");
  EXPECT_SCRIPT_EQ("|^(foo| (bar |baz)|", "backward('|(foo| (bar |baz)^|')");

  // nested parenthesis
  EXPECT_SCRIPT_EQ("^((foo))", "backward('^((foo))')");
  EXPECT_SCRIPT_EQ("^((foo))", "backward('(^(foo))')");
  EXPECT_SCRIPT_EQ("(^(foo))", "backward('((^foo))')");
  EXPECT_SCRIPT_EQ("(^(foo))", "backward('((f^oo))')");
  EXPECT_SCRIPT_EQ("(^(foo))", "backward('((f^oo))')");
  EXPECT_SCRIPT_EQ("(^(foo))", "backward('((fo^o))')");
  EXPECT_SCRIPT_EQ("(^(foo))", "backward('((foo^))')");
  EXPECT_SCRIPT_EQ("(^(foo))", "backward('((foo^))')");
  EXPECT_SCRIPT_EQ("(^(foo))", "backward('((foo)^)')");
  EXPECT_SCRIPT_EQ("^((foo))", "backward('((foo))^')");
}

TEST_F(TextPositionTest, moveWhile) {
  EXPECT_SCRIPT_VALID(
    "var doc = new Document('style');"
    "var range = new Range(doc);"
    "range.text = '   foo bar';"
    "var pos = new TextPosition(doc, 0);"
    "function whitespace() {"
    "  var char_code = this.charCode();"
    "  return Unicode.UCD[char_code].category.charAt(0) == 'Z';"
    "}");
  EXPECT_SCRIPT_EQ("1", "pos.moveWhile(whitespace).offset");
  EXPECT_SCRIPT_EQ("0", "pos.moveWhile(whitespace, -1).offset");
  EXPECT_SCRIPT_EQ("2", "pos.moveWhile(whitespace, 2).offset");
  EXPECT_SCRIPT_EQ("1", "pos.moveWhile(whitespace, -1).offset");
  EXPECT_SCRIPT_EQ("3", "pos.moveWhile(whitespace, Count.FORWARD).offset");
  EXPECT_SCRIPT_EQ("0", "pos.moveWhile(whitespace, Count.BACKWARD).offset");
}

}  // namespace
