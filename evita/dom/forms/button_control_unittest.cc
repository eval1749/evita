// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/abstract_dom_test.h"

namespace {

class ButtonControlTest : public dom::AbstractDomTest {
  protected: ButtonControlTest() {
  }
  public: virtual ~ButtonControlTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(ButtonControlTest);
};

TEST_F(ButtonControlTest, ctor) {
  EXPECT_SCRIPT_VALID("var sample = new ButtonControl('foo');");
  EXPECT_SCRIPT_FALSE("sample.disabled");
  EXPECT_SCRIPT_EQ("0", "sample.clientHeight");
  EXPECT_SCRIPT_EQ("0", "sample.clientLeft");
  EXPECT_SCRIPT_EQ("0", "sample.clientTop");
  EXPECT_SCRIPT_EQ("0", "sample.clientWidth");
  EXPECT_SCRIPT_EQ("foo", "sample.text");
}

TEST_F(ButtonControlTest, dispatchEvent) {
  EXPECT_SCRIPT_VALID(
      "var sample = new ButtonControl('foo');"
      "var clicked = false;"
      "sample.addEventListener('click', function() { clicked = true; });"
      "sample.dispatchEvent(new FormEvent('click'));");
  EXPECT_SCRIPT_EQ("true", "clicked");
}

TEST_F(ButtonControlTest, set_disabled) {
  EXPECT_SCRIPT_VALID(
      "var form = new Form();"
      "var sample = new ButtonControl('foo');"
      "form.add(sample);"
      "sample.disabled = true;");
  EXPECT_SCRIPT_TRUE("sample.disabled");
}

TEST_F(ButtonControlTest, set_clientLeft) {
  EXPECT_SCRIPT_VALID("var sample = new ButtonControl('foo');"
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

TEST_F(ButtonControlTest, set_text) {
  EXPECT_SCRIPT_VALID(
      "var sample = new ButtonControl('foo');"
      "sample.text = 'bar';");
  EXPECT_SCRIPT_EQ("bar", "sample.text");
}

}  // namespace
