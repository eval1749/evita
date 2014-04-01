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
  EXPECT_SCRIPT_EQ("true", "selection.collapsed");
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
      "  selection.focusOffset = focusOffset;"
      "  selection.extendTo(offset);"
      "  return selection.anchorOffset + ',' + selection.focusOffset +"
      "         ' ' + selection.start + ',' + selection.end;"
      "}");
  EXPECT_SCRIPT_EQ("4,2 2,4", "testIt(4, 4, 2)") << "Extend to left";
  EXPECT_SCRIPT_EQ("4,8 4,8", "testIt(4, 4, 8)") << "Extend to right";

  EXPECT_SCRIPT_EQ("4,2 2,4", "testIt(4, 6, 2)") << "Extend to left";
  EXPECT_SCRIPT_EQ("4,8 4,8", "testIt(4, 6, 8)") << "Extend to right";
  EXPECT_SCRIPT_FALSE("selection.collapsed");
}

TEST_F(TextFieldSelectionTest, set_text) {
  EXPECT_SCRIPT_VALID(
      "var sample = new TextFieldControl(123);"
      "var selection = sample.selection;"
      "function testIt(anchorOffset, focusOffset, new_text) {"
      "  selection.control.value = '0123456789';"
      "  selection.anchorOffset = anchorOffset;"
      "  selection.focusOffset = focusOffset;"
      "  selection.text = new_text;"
      "  return selection.control.value + ' ' + selection.anchorOffset +"
      "     ',' + selection.focusOffset;"
      "}");
  EXPECT_SCRIPT_EQ("0123foo456789 4,7", "testIt(4, 4, 'foo')") << "collapsed";
  EXPECT_SCRIPT_EQ("0123456789 4,4", "testIt(4, 4, '')") << "collapsed";
  EXPECT_SCRIPT_EQ("0123foobar89 4,10", "testIt(4, 8, 'foobar')") << "range";
  EXPECT_SCRIPT_EQ("012389 4,4", "testIt(4, 8, '')") << "range";
  EXPECT_SCRIPT_EQ("0123foobar89 10,4", "testIt(8, 4, 'foobar')") << "range";
  EXPECT_SCRIPT_EQ("012389 4,4", "testIt(8, 4, '')") << "range";
  EXPECT_SCRIPT_EQ("x 0,1", "testIt(0, 10, 'x')") << "range";
}

TEST_F(TextFieldSelectionTest, text) {
  EXPECT_SCRIPT_VALID(
      "var sample = new TextFieldControl(123);"
      "sample.value = '0123456789';"
      "var selection = sample.selection;"
      "function testIt(anchorOffset, focusOffset) {"
      "  selection.anchorOffset = anchorOffset;"
      "  selection.focusOffset = focusOffset;"
      "  return selection.text;"
      "}");
  EXPECT_SCRIPT_EQ("", "testIt(4, 4)") << "collapsed";
  EXPECT_SCRIPT_EQ("4567", "testIt(4, 8)") << "range";
}

}  // namespace
