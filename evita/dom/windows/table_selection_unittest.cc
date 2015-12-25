// Copyright (c) 2013 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/macros.h"
#include "evita/dom/testing/abstract_dom_test.h"
#include "evita/dom/mock_view_impl.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/script_host.h"
#include "gmock/gmock.h"

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
