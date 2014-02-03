// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "base/basictypes.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/ed_style.h"

namespace {

class StyleTest : public dom::AbstractDomTest {
  protected: StyleTest() {
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
  public: virtual ~StyleTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(StyleTest);
};

TEST_F(StyleTest, DefaultStyle) {
  EXPECT_SCRIPT_EQ("function", "typeof(Style)");
  EXPECT_SCRIPT_VALID(
    "var doc = new Document('style');"
    "var style = doc.styleAt(doc.length);");
  EXPECT_SCRIPT_EQ("16777215", "style.backgroundColor");
  EXPECT_SCRIPT_EQ("0", "style.charSyntax");
  EXPECT_SCRIPT_EQ("0", "style.color");
  EXPECT_SCRIPT_EQ("normal", "style.fontStyle");
  EXPECT_SCRIPT_EQ("normal", "style.fontWeight");
}

TEST_F(StyleTest, SetStyle) {
  EXPECT_SCRIPT_EQ("function", "typeof(Style)");
  EXPECT_SCRIPT_VALID(
    "var doc = new Document('style');"
    "var range = new Range(doc);"
    "range.text = 'foo';"
    "range.style({color: 0x00FF00, fontSize: 23});"
    "var style = doc.styleAt(1);");
  EXPECT_SCRIPT_EQ("ff00", "style.color.toString(16)");
  EXPECT_SCRIPT_EQ("23", "style.fontSize");
  EXPECT_SCRIPT_EQ("Error: Invalid style attribute name 'foo'",
                   "range.style({foo: 1})");
}

}  // namespace
