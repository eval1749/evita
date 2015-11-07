// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <string>

#include "base/basictypes.h"
#include "gmock/gmock.h"
#include "evita/dom/abstract_dom_test.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/script_host.h"
#include "evita/dom/view_delegate.h"

namespace dom {

using ::testing::_;

class TableSelectionTest : public AbstractDomTest {
 protected:
  TableSelectionTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(TableSelectionTest);
};

TEST_F(TableSelectionTest, Basic) {
  EXPECT_CALL(*mock_view_impl(), CreateTableWindow(_, _));
  EXPECT_SCRIPT_VALID(
      "var doc = Document.new('foo');"
      "var table_window = new TableWindow(doc);"
      "var sample = table_window.selection;");
  EXPECT_SCRIPT_TRUE("sample instanceof Selection");
  EXPECT_SCRIPT_TRUE("sample instanceof TableSelection");
}

TEST_F(TableSelectionTest, getRowStates) {
  EXPECT_CALL(*mock_view_impl(), CreateTableWindow(_, _));
  EXPECT_SCRIPT_VALID(
      "var doc = Document.new('foo');"
      "var table_window = new TableWindow(doc);"
      "var sample = table_window.selection;"
      "var result = sample.getRowStates(['a', 'b', 'c']);");
  EXPECT_SCRIPT_EQ("3", "result.length");
  EXPECT_SCRIPT_EQ("1", "result[1]");
}

}  // namespace dom
