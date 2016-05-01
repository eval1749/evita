// Copyright (c) 2013 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/macros.h"
#include "evita/css/style.h"
#include "evita/dom/testing/abstract_dom_test.h"

namespace {

class TextStyleTest : public dom::AbstractDomTest {
 public:
  ~TextStyleTest() override = default;

 protected:
  TextStyleTest() {
    auto const style = css::Style::Default();
    style->set_bgcolor(css::Color(255, 255, 255));
    style->set_color(css::Color(0x00, 0x00, 0x00));
    style->set_marker_color(css::Color(0x00, 0x99, 0x00));
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(TextStyleTest);
};

TEST_F(TextStyleTest, DefaultStyle) {
  EXPECT_SCRIPT_EQ("function", "typeof(TextStyle)");
  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('style');"
      "var style = doc.styleAt(doc.length);");
  EXPECT_SCRIPT_EQ("", "style.backgroundColor");
  EXPECT_SCRIPT_EQ("", "style.color");
  EXPECT_SCRIPT_EQ("", "style.fontStyle");
  EXPECT_SCRIPT_EQ("", "style.fontWeight");
  EXPECT_SCRIPT_EQ("", "style.syntax");
}

TEST_F(TextStyleTest, SetStyle) {
  EXPECT_SCRIPT_EQ("function", "typeof(TextStyle)");
  EXPECT_SCRIPT_VALID(
      "var doc = TextDocument.new('style');"
      "var range = new TextRange(doc);"
      "range.text = 'foo';"
      "range.setStyle({color: 0x00FF00, fontSize: 23});"
      "var style = doc.styleAt(1);");
  EXPECT_SCRIPT_EQ("ff00", "style.color.toString(16)");
  EXPECT_SCRIPT_EQ("23", "style.fontSize");
  EXPECT_SCRIPT_EQ(
      "Error: Failed to execute 'setStyle' on 'TextRange': Invalid style "
      "attribute name 'foo'",
      "range.setStyle({foo: 1})");
}

}  // namespace
