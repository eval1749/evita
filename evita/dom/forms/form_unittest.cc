// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"

#include "evita/dom/forms/form.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/public/api_event.h"
#include "evita/dom/public/view_event_handler.h"
#include "gmock/gmock.h"

namespace {

using ::testing::Eq;
using ::testing::_;

class FormTest : public dom::AbstractDomTest {
  protected: FormTest() {
  }
  public: virtual ~FormTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(FormTest);
};

TEST_F(FormTest, ctor) {
  EXPECT_CALL(*mock_view_impl(), CreateDialogBox(_));
  EXPECT_SCRIPT_VALID("var sample = new Form();");
  EXPECT_SCRIPT_TRUE("sample instanceof EventTarget");
  EXPECT_SCRIPT_EQ("0", "sample.controls.length");
}

TEST_F(FormTest, add) {
  EXPECT_CALL(*mock_view_impl(), CreateDialogBox(_));
  EXPECT_SCRIPT_VALID("var sample = new Form();"
                      "var text_field = new TextFieldControl(123);"
                      "sample.add(text_field);");
  EXPECT_SCRIPT_EQ("1", "sample.controls.length");
  EXPECT_SCRIPT_TRUE("sample.controls[0] === text_field");
}

TEST_F(FormTest, realize) {
  EXPECT_CALL(*mock_view_impl(), CreateDialogBox(_));
  EXPECT_CALL(*mock_view_impl(), RealizeDialogBox(Eq(1)));
  EXPECT_SCRIPT_VALID("var sample = new Form();"
                      "sample.realize();");
}

TEST_F(FormTest, show) {
  EXPECT_CALL(*mock_view_impl(), CreateDialogBox(_));
  EXPECT_CALL(*mock_view_impl(), RealizeDialogBox(Eq(1)));
  EXPECT_CALL(*mock_view_impl(), ShowDialogBox(Eq(1)));
  EXPECT_SCRIPT_VALID("var sample = new Form();"
                      "sample.realize();"
                      "sample.show();");
}

TEST_F(FormTest, DispatchFormEvent) {
  EXPECT_CALL(*mock_view_impl(), CreateDialogBox(_));
  EXPECT_SCRIPT_VALID("var sample = new Form();"
                      "var text_field = new TextFieldControl(123);"
                      "sample.add(text_field);");
  domapi::FormEvent form_event;
  form_event.target_id = 1;
  form_event.control_id = 123;
  form_event.type = L"change";
  form_event.data = L"foo";
  view_event_handler()->DispatchFormEvent(form_event);
  EXPECT_SCRIPT_EQ("foo", "text_field.value");
}

}  // namespace
