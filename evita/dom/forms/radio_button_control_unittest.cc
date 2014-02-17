// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/abstract_dom_test.h"

namespace {

class RadioButtonControlTest : public dom::AbstractDomTest {
  protected: RadioButtonControlTest() {
  }
  public: virtual ~RadioButtonControlTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(RadioButtonControlTest);
};

TEST_F(RadioButtonControlTest, ctor) {
  EXPECT_SCRIPT_VALID("var sample = new RadioButtonControl('foo', 123);");
  EXPECT_SCRIPT_EQ("123", "sample.controlId");
  EXPECT_SCRIPT_EQ("false", "sample.checked");
  EXPECT_SCRIPT_VALID("sample.checked = true;");
  EXPECT_SCRIPT_EQ("true", "sample.checked");
}

TEST_F(RadioButtonControlTest, dispatchEvent) {
  EXPECT_SCRIPT_VALID(
      "var form1 = new Form();"
      "var sample1 = new RadioButtonControl('foo', 123);"
      "var sample2 = new RadioButtonControl('foo', 124);"
      "form1.add(sample1);"
      "form1.add(sample2);");

  // No radio buttons are checked.
  EXPECT_SCRIPT_EQ("false", "sample1.checked");
  EXPECT_SCRIPT_EQ("false", "sample2.checked");

  // Check |sample1|
  EXPECT_SCRIPT_VALID("sample1.dispatchEvent(new FormEvent('change'));");
  EXPECT_SCRIPT_EQ("true", "sample1.checked");
  EXPECT_SCRIPT_EQ("false", "sample2.checked");

  // Check |sample2|
  EXPECT_SCRIPT_VALID("sample2.dispatchEvent(new FormEvent('change'));");
  EXPECT_SCRIPT_EQ("false", "sample1.checked");
  EXPECT_SCRIPT_EQ("true", "sample2.checked");
}

TEST_F(RadioButtonControlTest, set_checked) {
  EXPECT_SCRIPT_VALID(
      "var form1 = new Form();"
      "var sample1 = new RadioButtonControl('foo', 123);"
      "var sample2 = new RadioButtonControl('foo', 124);"
      "form1.add(sample1);"
      "form1.add(sample2);");

  // No radio buttons are checked.
  EXPECT_SCRIPT_EQ("false", "sample1.checked");
  EXPECT_SCRIPT_EQ("false", "sample2.checked");

  // Check |sample1|
  EXPECT_SCRIPT_VALID("sample1.checked = true;");
  EXPECT_SCRIPT_EQ("true", "sample1.checked");
  EXPECT_SCRIPT_EQ("false", "sample2.checked");

  // Check |sample2|
  EXPECT_SCRIPT_VALID("sample2.checked = true;");
  EXPECT_SCRIPT_EQ("false", "sample1.checked");
  EXPECT_SCRIPT_EQ("true", "sample2.checked");
}

}  // namespace
