// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/mock_view_impl.h"

namespace {

class CheckboxControlTest : public dom::AbstractDomTest {
  protected: CheckboxControlTest() {
  }
  public: virtual ~CheckboxControlTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(CheckboxControlTest);
};

TEST_F(CheckboxControlTest, ctor) {
  EXPECT_SCRIPT_VALID("var sample = new CheckboxControl();");
  EXPECT_SCRIPT_FALSE("sample.disabled");
  EXPECT_SCRIPT_EQ("false", "sample.checked");
  EXPECT_SCRIPT_FALSE("sample.checked");
}

TEST_F(CheckboxControlTest, dispatchEvent) {
  EXPECT_SCRIPT_VALID(
      "var sample = new CheckboxControl();"
      "var changed = 0;"
      "sample.addEventListener('change', function() { ++changed; });"
      "sample.dispatchEvent(new FormEvent('change', {data: 'on'}));");
  EXPECT_SCRIPT_EQ("true", "sample.checked");
  EXPECT_SCRIPT_EQ("1", "changed") << "UI changes value.";

  EXPECT_SCRIPT_VALID("sample.dispatchEvent(new FormEvent('change'))");
  EXPECT_SCRIPT_EQ("2", "changed") << "UI changes control's value.";

  EXPECT_SCRIPT_VALID("sample.dispatchEvent(new FormEvent('change'))");
  EXPECT_SCRIPT_EQ("2", "changed") <<
      "UI doesn't change value, but event was dispatched.";
}

TEST_F(CheckboxControlTest, set_value) {
  EXPECT_SCRIPT_VALID(
      "var form = new Form();"
      "var sample = new CheckboxControl();"
      "form.add(sample);"
      "var changed = 0;"
      "sample.addEventListener('change', function() { ++changed; });"
      "sample.checked = true;");
  EXPECT_SCRIPT_EQ("1", "changed") << "Change value by script";

  EXPECT_SCRIPT_VALID("sample.checked = true");
  EXPECT_SCRIPT_EQ("1", "changed") << "Script doesn't change value.";
}

TEST_F(CheckboxControlTest, set_disabled) {
  EXPECT_SCRIPT_VALID(
      "var form = new Form();"
      "var sample = new CheckboxControl();"
      "form.add(sample);"
      "sample.disabled = true;");
  EXPECT_SCRIPT_TRUE("sample.disabled");
}


TEST_F(CheckboxControlTest, set_clientLeft) {
  EXPECT_SCRIPT_VALID("var sample = new CheckboxControl();"
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
