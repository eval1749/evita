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
  EXPECT_EQ("true", RunScript("sample instanceof Selection"));
  EXPECT_EQ("false", RunScript("sample instanceof Range"));
  EXPECT_EQ("true", RunScript("sample.document == doc"));
  EXPECT_EQ("0", RunScript("sample.start"));
  EXPECT_EQ("0", RunScript("sample.end"));
}

}  // namespace
