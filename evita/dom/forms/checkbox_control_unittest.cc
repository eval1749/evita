// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/mock_view_impl.h"
#include "gmock/gmock.h"

namespace {

using ::testing::Eq;
using ::testing::_;

class CheckboxControlTest : public dom::AbstractDomTest {
  protected: CheckboxControlTest() {
  }
  public: virtual ~CheckboxControlTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(CheckboxControlTest);
};

TEST_F(CheckboxControlTest, ctor) {
  EXPECT_SCRIPT_VALID("var sample = new CheckboxControl(123);");
  EXPECT_SCRIPT_EQ("123", "sample.controlId");
  EXPECT_SCRIPT_FALSE("sample.disabled");
  EXPECT_SCRIPT_EQ("false", "sample.checked");
  EXPECT_SCRIPT_FALSE("sample.checked");
}

TEST_F(CheckboxControlTest, dispatchEvent) {
  EXPECT_SCRIPT_VALID(
      "var sample = new CheckboxControl(123);"
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
  EXPECT_CALL(*mock_view_impl(), CreateDialogBox(_));
  EXPECT_CALL(*mock_view_impl(), DidChangeFormContents(Eq(1)));
  EXPECT_SCRIPT_VALID(
      "var form = new Form('form1');"
      "var sample = new CheckboxControl(123);"
      "form.add(sample);"
      "var changed = 0;"
      "sample.addEventListener('change', function() { ++changed; });"
      "sample.checked = true;");
  EXPECT_SCRIPT_EQ("1", "changed") << "Change value by script";

  EXPECT_SCRIPT_VALID("sample.checked = true");
  EXPECT_SCRIPT_EQ("1", "changed") << "Script doesn't change value.";
}

TEST_F(CheckboxControlTest, set_disabled) {
  EXPECT_CALL(*mock_view_impl(), CreateDialogBox(_));
  EXPECT_CALL(*mock_view_impl(), DidChangeFormContents(Eq(1)));
  EXPECT_SCRIPT_VALID(
      "var form = new Form('form1');"
      "var sample = new CheckboxControl(123);"
      "form.add(sample);"
      "sample.disabled = true;");
  EXPECT_SCRIPT_TRUE("sample.disabled");
}

}  // namespace
