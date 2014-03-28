// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/abstract_dom_test.h"

namespace {

class TextFieldSelectionTest : public dom::AbstractDomTest {
  protected: TextFieldSelectionTest() {
  }
  public: virtual ~TextFieldSelectionTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(TextFieldSelectionTest);
};

TEST_F(TextFieldSelectionTest, ctor) {
  EXPECT_SCRIPT_VALID("var sample = new TextFieldControl(123);"
                      "var selection = sample.selection;");
  EXPECT_SCRIPT_TRUE("selection instanceof TextFieldSelection");
  EXPECT_SCRIPT_EQ("0", "selection.anchorOffset");
  EXPECT_SCRIPT_EQ("0", "selection.end");
  EXPECT_SCRIPT_EQ("0", "selection.focusOffset");
  EXPECT_SCRIPT_EQ("0", "selection.start");
}

TEST_F(TextFieldSelectionTest, collapseTo) {
  EXPECT_SCRIPT_VALID("var sample = new TextFieldControl(123);"
                      "sample.value = '0123456789';"
                      "var selection = sample.selection;"
                      "selection.collapseTo(3)");
  EXPECT_SCRIPT_EQ("3,3",
      "selection.anchorOffset + ',' + selection.focusOffset");
  EXPECT_SCRIPT_VALID("selection.focusOffset = 5; selection.collapseTo(8);");
  EXPECT_SCRIPT_EQ("8,8",
      "selection.anchorOffset+ ',' + selection.focusOffset");
}

TEST_F(TextFieldSelectionTest, extendTo) {
  EXPECT_SCRIPT_VALID(
      "var sample = new TextFieldControl(123);"
      "sample.value = '0123456789';"
      "var selection = sample.selection;"
      "function testIt(anchorOffset, focusOffset, offset) {"
      "  selection.anchorOffset = anchorOffset;"
      "  selection.focsuOffset = focusOffset;"
      "  selection.extendTo(offset);"
      "  return selection.anchorOffset + ',' + selection.focusOffset +"
      "         ' ' + selection.start + ',' + selection.end;"
      "}");
  EXPECT_SCRIPT_EQ("4,2 2,4", "testIt(4, 4, 2)") << "Extend to left";
  EXPECT_SCRIPT_EQ("4,8 4,8", "testIt(4, 4, 8)") << "Extend to right";

  EXPECT_SCRIPT_EQ("4,2 2,4", "testIt(4, 6, 2)") << "Extend to left";
  EXPECT_SCRIPT_EQ("4,8 4,8", "testIt(4, 6, 8)") << "Extend to right";
}

}  // namespace
