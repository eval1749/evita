// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/abstract_dom_test.h"

#include "evita/dom/mock_view_impl.h"

namespace {

class LabelControlTest : public dom::AbstractDomTest {
  protected: LabelControlTest() {
  }
  public: virtual ~LabelControlTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(LabelControlTest);
};

TEST_F(LabelControlTest, ctor) {
  EXPECT_SCRIPT_VALID("var sample = new LabelControl('foo');");
  EXPECT_SCRIPT_EQ("foo", "sample.text");
  EXPECT_SCRIPT_FALSE("sample.disabled");
  EXPECT_SCRIPT_EQ("0", "sample.clientHeight");
  EXPECT_SCRIPT_EQ("0", "sample.clientLeft");
  EXPECT_SCRIPT_EQ("0", "sample.clientTop");
  EXPECT_SCRIPT_EQ("0", "sample.clientWidth");
}

TEST_F(LabelControlTest, dispatchEvent) {
  EXPECT_SCRIPT_VALID(
      "var sample = new LabelControl('foo');"
      "var clicked = false;"
      "sample.addEventListener('click', function() { clicked = true; });"
      "sample.dispatchEvent(new FormEvent('click'));");
  EXPECT_SCRIPT_EQ("true", "clicked");
}

TEST_F(LabelControlTest, set_disabled) {
  EXPECT_SCRIPT_VALID(
      "var form = new Form('form1');"
      "var sample = new LabelControl('foo');"
      "form.add(sample);"
      "sample.disabled = true;");
  EXPECT_SCRIPT_TRUE("sample.disabled");
}

TEST_F(LabelControlTest, set_text) {
  EXPECT_SCRIPT_VALID("var sample = new LabelControl('foo');");
  EXPECT_SCRIPT_VALID("sample.text = 'bar'");
  EXPECT_SCRIPT_EQ("bar", "sample.text");
}

TEST_F(LabelControlTest, setRect) {
  EXPECT_SCRIPT_VALID("var sample = new LabelControl('foo');");
  EXPECT_SCRIPT_VALID("sample.setRect(1, 2, 3, 4)");
  EXPECT_SCRIPT_EQ("4", "sample.clientHeight");
  EXPECT_SCRIPT_EQ("1", "sample.clientLeft");
  EXPECT_SCRIPT_EQ("2", "sample.clientTop");
  EXPECT_SCRIPT_EQ("3", "sample.clientWidth");
}

}  // namespace
