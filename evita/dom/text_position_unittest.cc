// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/basictypes.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/ed_style.h"

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
  EXPECT_SCRIPT_EQ("3", "pos.moveWhile(whitespace).offset");
  EXPECT_SCRIPT_EQ("0", "pos.moveWhile(whitespace, -1).offset");
}

}  // namespace
