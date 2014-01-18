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

class TextSelectionTest : public dom::AbstractDomTest {
  protected: TextSelectionTest() {
  }
  public: virtual ~TextSelectionTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(TextSelectionTest);
};

TEST_F(TextSelectionTest, active) {
  EXPECT_CALL(*mock_view_impl(), CreateTextWindow(_));
  EXPECT_VALID_SCRIPT(
      "var doc = new Document('foo');"
      "var range = new Range(doc);"
      "var text_window = new TextWindow(range);"
      "var sample = text_window.selection;"
      "sample.range.text = 'foo';");
  EXPECT_SCRIPT_EQ("3", "sample.active");
  EXPECT_VALID_SCRIPT("sample.startIsActive = true;");
  EXPECT_SCRIPT_EQ("0", "sample.active");
}

TEST_F(TextSelectionTest, Realize) {
  EXPECT_CALL(*mock_view_impl(), CreateTextWindow(_));
  EXPECT_VALID_SCRIPT(
      "var doc = new Document('foo');"
      "var range = new Range(doc);"
      "var text_window = new TextWindow(range);"
      "var sample = text_window.selection");
  EXPECT_SCRIPT_TRUE("sample instanceof TextSelection");
  EXPECT_SCRIPT_FALSE("sample instanceof Range");
  EXPECT_SCRIPT_TRUE("sample.document == doc");
  EXPECT_SCRIPT_EQ("0", "sample.range.start");
  EXPECT_SCRIPT_EQ("0", "sample.range.end");
  EXPECT_SCRIPT_FALSE("sample.startIsActive");
  EXPECT_VALID_SCRIPT("sample.startIsActive = true;");
  EXPECT_SCRIPT_TRUE("sample.startIsActive");
}

}  // namespace
