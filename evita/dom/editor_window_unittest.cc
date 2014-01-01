// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/basictypes.h"
#include "gmock/gmock.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"

namespace {

using ::testing::_;

class EditorWindowTest : public dom::AbstractDomTest {
  protected: EditorWindowTest() {
  }
  public: virtual ~EditorWindowTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(EditorWindowTest);
};

TEST_F(EditorWindowTest, Realize) {
  EXPECT_CALL(*mock_view_impl(), CreateEditorWindow(::testing::_));
  RunScript("var sample = new EditorWindow()");
  EXPECT_EQ("true", RunScript("sample instanceof EditorWindow"));
  EXPECT_EQ("true", RunScript("sample instanceof Window"));
  EXPECT_EQ("1", RunScript("sample.id"));

  EXPECT_CALL(*mock_view_impl(), RealizeWindow(::testing::Eq(1)));
  RunScript("sample.realize()");
}

}  // namespace
