// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/mock_view_impl.h"

namespace {

class RadioButtonControlTest : public dom::AbstractDomTest {
  protected: RadioButtonControlTest() {
  }
  public: virtual ~RadioButtonControlTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(RadioButtonControlTest);
};

TEST_F(RadioButtonControlTest, ctor) {
  EXPECT_SCRIPT_VALID("var sample = new RadioButtonControl('foo');");
  EXPECT_SCRIPT_EQ("123", "sample.controlId");
  EXPECT_SCRIPT_FALSE("sample.disabled");
  EXPECT_SCRIPT_EQ("false", "sample.checked");
  EXPECT_SCRIPT_FALSE("sample.checked");
}

TEST_F(RadioButtonControlTest, dispatchEvent) {
  EXPECT_SCRIPT_VALID(
      "var form1 = new Form();"
      "var sample1 = new RadioButtonControl('foo');"
      "var sample2 = new RadioButtonControl('foo');"
      "form1.add(sample1);"
      "form1.add(sample2);"
      "sample1.changed = 0;"
      "sample2.changed = 0;"
      "function changed(event) { ++event.target.changed; }"
      "sample1.addEventListener('change', changed);"
      "sample2.addEventListener('change', changed);");

  // No radio buttons are checked.
  EXPECT_SCRIPT_EQ("false", "sample1.checked");
  EXPECT_SCRIPT_EQ("false", "sample2.checked");

  // Check |sample1|
  EXPECT_SCRIPT_VALID("sample1.dispatchEvent(new FormEvent('change'));");
  EXPECT_SCRIPT_EQ("true", "sample1.checked");
  EXPECT_SCRIPT_EQ("false", "sample2.checked");
  EXPECT_SCRIPT_EQ("1", "sample1.changed");
  EXPECT_SCRIPT_EQ("0", "sample2.changed");

  // Check |sample2|
  EXPECT_SCRIPT_VALID("sample2.dispatchEvent(new FormEvent('change'));");
  EXPECT_SCRIPT_EQ("false", "sample1.checked");
  EXPECT_SCRIPT_EQ("true", "sample2.checked");
  EXPECT_SCRIPT_EQ("2", "sample1.changed");
  EXPECT_SCRIPT_EQ("1", "sample2.changed");
}

TEST_F(RadioButtonControlTest, set_checked) {
  EXPECT_SCRIPT_VALID(
      "var form1 = new Form();"
      "var sample1 = new RadioButtonControl('foo');"
      "var sample2 = new RadioButtonControl('foo');"
      "form1.add(sample1);"
      "form1.add(sample2);"
      "sample1.changed = 0;"
      "sample2.changed = 0;"
      "function changed(event) { ++event.target.changed; }"
      "sample1.addEventListener('change', changed);"
      "sample2.addEventListener('change', changed);");

  // No radio buttons are checked.
  EXPECT_SCRIPT_EQ("false", "sample1.checked");
  EXPECT_SCRIPT_EQ("false", "sample2.checked");

  // Check |sample1|
  EXPECT_SCRIPT_VALID("sample1.checked = true;");
  EXPECT_SCRIPT_EQ("true", "sample1.checked");
  EXPECT_SCRIPT_EQ("false", "sample2.checked");
  EXPECT_SCRIPT_EQ("1", "sample1.changed");
  EXPECT_SCRIPT_EQ("0", "sample2.changed");

  // Check |sample2|
  EXPECT_SCRIPT_VALID("sample2.checked = true;");
  EXPECT_SCRIPT_EQ("false", "sample1.checked");
  EXPECT_SCRIPT_EQ("true", "sample2.checked");
  EXPECT_SCRIPT_EQ("2", "sample1.changed");
  EXPECT_SCRIPT_EQ("1", "sample2.changed");
}

TEST_F(RadioButtonControlTest, set_disabled) {
  EXPECT_SCRIPT_VALID(
      "var form = new Form();"
      "var sample = new RadioButtonControl('foo');"
      "form.add(sample);"
      "sample.disabled = true;");
  EXPECT_SCRIPT_TRUE("sample.disabled");
}


TEST_F(RadioButtonControlTest, set_clientLeft) {
  EXPECT_SCRIPT_VALID("var sample = new RadioButtonControl('foo');"
                      "sample.clientLeft = 1;"
                      "sample.clientTop = 2;"
                      "sample.clientWidth = 3;"
                      "sample.clientHeight = 4;");
  EXPECT_SCRIPT_EQ("4", "sample.clientHeight");
  EXPECT_SCRIPT_EQ("1", "sample.clientLeft");
  EXPECT_SCRIPT_EQ("2", "sample.clientTop");
  EXPECT_SCRIPT_EQ("3", "sample.clientWidth");

  EXPECT_SCRIPT_VALID("sample.clientLeft = 10");
  EXPECT_SCRIPT_EQ("3", "sample.clientWidth");

  EXPECT_SCRIPT_VALID("sample.clientWidth = 30");
  EXPECT_SCRIPT_EQ("30", "sample.clientWidth");

  EXPECT_SCRIPT_VALID("sample.clientTop = 20");
  EXPECT_SCRIPT_EQ("4", "sample.clientHeight");

  EXPECT_SCRIPT_VALID("sample.clientHeight = 40");
  EXPECT_SCRIPT_EQ("40", "sample.clientHeight");
}

}  // namespace
