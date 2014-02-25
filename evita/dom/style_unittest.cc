// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "base/basictypes.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/text/style.h"

namespace {

class StyleTest : public dom::AbstractDomTest {
  protected: StyleTest() {
    auto const style = text::StyleValues::Default();
    style->set_bgcolor(text::Color(255, 255, 255));
    style->set_color(text::Color(0x00, 0x00, 0x00));
    style->set_marker_color(text::Color(0x00, 0x99, 0x00));
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
