// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/abstract_dom_test.h"

namespace {

class CheckboxControlTest : public dom::AbstractDomTest {
  protected: CheckboxControlTest() {
  }
  public: virtual ~CheckboxControlTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(CheckboxControlTest);
};

TEST_F(CheckboxControlTest, ctor) {
  EXPECT_SCRIPT_VALID("var sample = new CheckboxControl(123);");
  EXPECT_SCRIPT_EQ("123", "sample.resouce_id");
  EXPECT_SCRIPT_EQ("false", "sample.checked");
  EXPECT_SCRIPT_VALID("sample.checked = true;");
  EXPECT_SCRIPT_EQ("true", "sample.checked");
}

TEST_F(CheckboxControlTest, dispatchEvent) {
  EXPECT_SCRIPT_VALID(
      "var sample = new CheckboxControl(123);"
      "var event = new FormEvent('change', {data: 'on'});"
      "sample.dispatchEvent(event);");
  EXPECT_SCRIPT_EQ("true", "sample.checked");
}

}  // namespace
