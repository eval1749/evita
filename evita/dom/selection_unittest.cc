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

class SelectionTest : public dom::AbstractDomTest {
  protected: SelectionTest() {
  }
  public: virtual ~SelectionTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(SelectionTest);
};

TEST_F(SelectionTest, Realize) {
  EXPECT_CALL(*mock_view_impl(), CreateTextWindow(::testing::_));
  RunScript("var doc = new Document('foo');"
            "var range = new Range(doc);"
            "var text_window = new TextWindow(range);"
            "var sample = text_window.selection");
  EXPECT_SCRIPT_TRUE("sample instanceof Selection");
  EXPECT_SCRIPT_FALSE("sample instanceof Range");
  EXPECT_SCRIPT_TRUE("sample.document == doc");
  EXPECT_SCRIPT_EQ("0", "sample.range.start");
  EXPECT_SCRIPT_EQ("0", "sample.range.end");
  EXPECT_SCRIPT_FALSE("sample.start_is_active");
  RunScript("sample.start_is_active = true;");
  EXPECT_SCRIPT_TRUE("sample.start_is_active");
}

}  // namespace
