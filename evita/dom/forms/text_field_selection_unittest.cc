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
  EXPECT_SCRIPT_EQ("0", "selection.end");
  EXPECT_SCRIPT_EQ("0", "selection.start");
  EXPECT_SCRIPT_EQ("false", "selection.startIsActive");
}

TEST_F(TextFieldSelectionTest, setRange) {
  EXPECT_SCRIPT_VALID("var sample = new TextFieldControl(123);"
                      "sample.value = '0123456789';"
                      "var selection = sample.selection;"
                      "selection.setRange(4, 7, true)");
  EXPECT_SCRIPT_EQ("7", "selection.end");
  EXPECT_SCRIPT_EQ("4", "selection.start");
  EXPECT_SCRIPT_EQ("true", "selection.startIsActive");

  EXPECT_SCRIPT_EQ("5", "selection.start = 5; selection.start");
  EXPECT_SCRIPT_EQ("8", "selection.end = 8; selection.end");
  EXPECT_SCRIPT_EQ("false", "selection.startIsActive= false;"
                            "selection.startIsActive");

  EXPECT_SCRIPT_EQ("4", "selection.end = 4; selection.start") <<
      "Collapse to start if end is less than start.";

  EXPECT_SCRIPT_EQ("7", "selection.start = 7; selection.end") <<
      "Collapse to end if start is grater than start.";
}

}  // namespace
