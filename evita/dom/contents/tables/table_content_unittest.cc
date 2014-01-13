// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/abstract_dom_test.h"

namespace {

class TableContentTest : public dom::AbstractDomTest {
  protected: TableContentTest() {
  }
  public: virtual ~TableContentTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(TableContentTest);
};

TEST_F(TableContentTest, Basic) {
  EXPECT_VALID_SCRIPT(
    "var table = new TableContent('foo');"
    "table.headerRow.addCell('Name', 'Color', 'Size');"
    "table.addCell('Foo', 'Blue', '42');"
    "var change_count = table.changeCount;");
  EXPECT_SCRIPT_TRUE("change_count != 0");
  EXPECT_VALID_SCRIPT(
    "table.addCell('Bar', 'Red', '35');"
    "table.addCell('Baz', 'Geeen', '18');"
    "table.addCell('Quux', 'White', '25');");
  EXPECT_SCRIPT_TRUE("change_count != table.changeCount");
  EXPECT_SCRIPT_EQ("4", "table.length");
  EXPECT_SCRIPT_EQ("3", "table.row(1).length");
  EXPECT_SCRIPT_EQ("Foo", "table.cell(0, 0).text");
  EXPECT_SCRIPT_EQ("Bar", "table.cell(0, 1).text");
  EXPECT_SCRIPT_EQ("Red", "table.cell(1, 1).text");
  EXPECT_SCRIPT_EQ("18", "table.cell(2, 2).text");
}

}  // namespace
