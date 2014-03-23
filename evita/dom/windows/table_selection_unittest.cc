// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/basictypes.h"
#include "gmock/gmock.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/script_host.h"
#include "evita/dom/view_delegate.h"

namespace {

using ::testing::_;

class TableSelectionTest : public dom::AbstractDomTest {
  protected: TableSelectionTest() {
  }
  public: virtual ~TableSelectionTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(TableSelectionTest);
};

TEST_F(TableSelectionTest, Basic) {
  EXPECT_CALL(*mock_view_impl(), CreateTableWindow(_, _));
  EXPECT_SCRIPT_VALID(
      "var doc = new Document('foo');"
      "var table_window = new TableWindow(doc);"
      "var sample = table_window.selection;");
  EXPECT_SCRIPT_TRUE("sample instanceof Selection");
  EXPECT_SCRIPT_TRUE("sample instanceof TableSelection");
}

TEST_F(TableSelectionTest, getRowStates) {
  EXPECT_CALL(*mock_view_impl(), CreateTableWindow(_, _));
  EXPECT_SCRIPT_VALID(
      "var doc = new Document('foo');"
      "var table_window = new TableWindow(doc);"
      "var sample = table_window.selection;"
      "var result = sample.getRowStates(['a', 'b', 'c']);");
  EXPECT_SCRIPT_EQ("3", "result.length");
  EXPECT_SCRIPT_EQ("1", "result[1]");
}

}  // namespace
