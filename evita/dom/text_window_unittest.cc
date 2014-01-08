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

using ::testing::Eq;
using ::testing::_;

class TextWindowTest : public dom::AbstractDomTest {
  protected: TextWindowTest() {
  }
  public: virtual ~TextWindowTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(TextWindowTest);
};

TEST_F(TextWindowTest, Realize) {
  EXPECT_CALL(*mock_view_impl(), CreateTextWindow(_));
  RunScript("var doc = new Document('foo');"
            "var range = new Range(doc);"
            "var sample = new TextWindow(range);");
  EXPECT_SCRIPT_TRUE("sample instanceof TextWindow");
  EXPECT_SCRIPT_TRUE("sample instanceof Window");
  EXPECT_SCRIPT_EQ("1", "sample.id");
  EXPECT_SCRIPT_TRUE("sample.document == doc");
  EXPECT_SCRIPT_TRUE("sample.selection instanceof Selection");

  EXPECT_CALL(*mock_view_impl(), RealizeWindow(Eq(1)));
  RunScript("sample.realize()");
}

}  // namespace
