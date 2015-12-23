// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/testing/abstract_dom_test.h"

#include "evita/dom/forms/text_field_control.h"
#include "evita/dom/mock_view_impl.h"

namespace dom {

class TextFieldControlTest : public AbstractDomTest {
 public:
  ~TextFieldControlTest() override = default;

 protected:
  TextFieldControlTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(TextFieldControlTest);
};

TEST_F(TextFieldControlTest, ctor) {
  EXPECT_SCRIPT_VALID("var sample = new TextFieldControl();");
  EXPECT_SCRIPT_FALSE("sample.disabled");
  EXPECT_SCRIPT_EQ("", "sample.value");
}

TEST_F(TextFieldControlTest, set_disabled) {
  EXPECT_SCRIPT_VALID(
      "var form = new Form();"
      "var sample = new TextFieldControl();"
      "form.add(sample);"
      "sample.disabled = true;");
  EXPECT_SCRIPT_TRUE("sample.disabled");
}

TEST_F(TextFieldControlTest, set_clientLeft) {
  EXPECT_SCRIPT_VALID(
      "var sample = new TextFieldControl();"
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

TEST_F(TextFieldControlTest, set_value) {
  EXPECT_SCRIPT_VALID(
      "var form = new Form();"
      "var sample = new TextFieldControl();"
      "form.add(sample);"
      "var num_changes = 0;"
      "var num_inputs = 0;"
      "sample.addEventListener('change', function() { ++num_changes; });"
      "sample.addEventListener('input', function() { ++num_inputs; });"
      "sample.value = 'foo';");
  EXPECT_SCRIPT_EQ("0", "num_changes")
      << "Change value from script doesn't dispatch 'change' event.";
  EXPECT_SCRIPT_EQ("0", "num_inputs")
      << "Change value from script doesn't dispatch 'input' event.";
}

}  // namespace dom
