// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"

#include "evita/dom/forms/form.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/public/view_event.h"
#include "evita/dom/public/view_event_handler.h"

namespace dom {

using ::testing::Eq;
using ::testing::_;

class FormTest : public AbstractDomTest {
 public:
  ~FormTest() override = default;

 protected:
  FormTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(FormTest);
};

TEST_F(FormTest, ctor) {
  EXPECT_SCRIPT_VALID("var sample = new Form();");
  EXPECT_SCRIPT_TRUE("sample instanceof EventTarget");
  EXPECT_SCRIPT_EQ("0", "sample.height");
  EXPECT_SCRIPT_EQ("0", "sample.controls.length");
  EXPECT_SCRIPT_TRUE("sample.focusControl == null");
  EXPECT_SCRIPT_TRUE("sample.title == ''");
  EXPECT_SCRIPT_EQ("0", "sample.width");
}

TEST_F(FormTest, add) {
  EXPECT_SCRIPT_VALID(
      "var sample = new Form();"
      "var text_field = new TextFieldControl();"
      "sample.add(text_field);");
  EXPECT_SCRIPT_EQ("1", "sample.controls.length");
  EXPECT_SCRIPT_TRUE("sample.controls[0] === text_field");
}

TEST_F(FormTest, focusControl) {
  EXPECT_SCRIPT_VALID(
      "var sample = new Form();"
      "var button = new ButtonControl('button');"
      "sample.focusControl = button");
  EXPECT_SCRIPT_TRUE("sample.focusControl == button");
  EXPECT_SCRIPT_VALID("sample.focusControl = null")
      << "Set null to Form.protected.focusControl means the form doesn't have"
         " focus control.";
  EXPECT_SCRIPT_TRUE("sample.focusControl == null");
}

TEST_F(FormTest, height) {
  EXPECT_SCRIPT_VALID(
      "var sample = new Form();"
      "sample.height = 123");
  EXPECT_SCRIPT_EQ("123", "sample.height");
}

TEST_F(FormTest, title) {
  EXPECT_SCRIPT_VALID(
      "var sample = new Form();"
      "sample.title = 'foo'");
  EXPECT_SCRIPT_EQ("foo", "sample.title");
}

TEST_F(FormTest, width) {
  EXPECT_SCRIPT_VALID(
      "var sample = new Form();"
      "sample.width = 123");
  EXPECT_SCRIPT_EQ("123", "sample.width");
}

}  // namespace dom
