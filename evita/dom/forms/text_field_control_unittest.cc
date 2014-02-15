// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"

#include "evita/dom/forms/text_field_control.h"

namespace {

class TextFieldControlTest : public dom::AbstractDomTest {
  protected: TextFieldControlTest() {
  }
  public: virtual ~TextFieldControlTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(TextFieldControlTest);
};

TEST_F(TextFieldControlTest, ctor) {
  EXPECT_SCRIPT_VALID("var sample = new TextFieldControl(123);");
  EXPECT_SCRIPT_EQ("123", "sample.controlId");
  EXPECT_SCRIPT_EQ("", "sample.value");
  EXPECT_SCRIPT_VALID("sample.value = 'foo';");
  EXPECT_SCRIPT_EQ("foo", "sample.value");
}

TEST_F(TextFieldControlTest, dispatchEvent) {
  EXPECT_SCRIPT_VALID(
      "var sample = new TextFieldControl(123);"
      "var event = new FormEvent('change', {data: 'foo'});"
      "sample.dispatchEvent(event);");
  EXPECT_SCRIPT_EQ("foo", "sample.value");
}

}  // namespace
