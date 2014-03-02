// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"

#include "evita/dom/forms/text_field_control.h"
#include "evita/dom/mock_view_impl.h"
#include "gmock/gmock.h"

namespace {

using ::testing::Eq;
using ::testing::_;

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
}

TEST_F(TextFieldControlTest, dispatchEvent) {
  EXPECT_SCRIPT_VALID(
      "var sample = new TextFieldControl(123);"
      "var changed = 0;"
      "sample.addEventListener('change', function() { ++changed; });"
      "var event = new FormEvent('change', {data: 'foo'});"
      "sample.dispatchEvent(event);");
  EXPECT_SCRIPT_EQ("foo", "sample.value");
  EXPECT_SCRIPT_EQ("1", "changed") << "UI changes value.";

  EXPECT_SCRIPT_VALID("sample.dispatchEvent(new FormEvent('change'))");
  EXPECT_SCRIPT_EQ("2", "changed") << "UI set empty value.";

  EXPECT_SCRIPT_VALID("sample.dispatchEvent(new FormEvent('change'))");
  EXPECT_SCRIPT_EQ("2", "changed") <<
      "UI doesn't change value, but event was dispatched.";
}

TEST_F(TextFieldControlTest, set_value) {
  EXPECT_CALL(*mock_view_impl(), CreateDialogBox(_));
  EXPECT_CALL(*mock_view_impl(), DidChangeFormContents(Eq(1)));
  EXPECT_SCRIPT_VALID(
      "var form = new Form();"
      "var sample = new TextFieldControl(123);"
      "form.add(sample);"
      "var changed = 0;"
      "sample.addEventListener('change', function() { ++changed; });"
      "sample.value = 'foo';");
  EXPECT_SCRIPT_EQ("1", "changed") << "Script changes value.";

  EXPECT_SCRIPT_VALID("sample.value = 'foo'");
  EXPECT_SCRIPT_EQ("1", "changed") << "Script doesn't change value.";
}

}  // namespace
