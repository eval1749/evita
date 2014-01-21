// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include <memory>

#pragma warning(push)
#pragma warning(disable: 4365 4625 4626 4826)
#include "gtest/gtest.h"
#pragma warning(pop)

#include "evita/ui/controls/table_model.h"

namespace {

class TableModeTest : public ::testing::Test {
  protected: TableModeTest() {
  }
  public: virtual ~TableModeTest() {
  }

  DISALLOW_COPY_AND_ASSIGN(TableModeTest);
};

TEST_F(TableModeTest, AddRow) {
  std::unique_ptr<ui::TableModel> table_model(new ui::TableModel());
  table_model->SetHeaderRow(L"foo\t\bar\tbaz");
  EXPECT_EQ(3, table_model->header_row()->length());
  table_model->AddRow(L"one\t10\t11");
  table_model->AddRow(L"two\t20\t21\t22\t24");
  table_model->AddRow(L"three\t30");
  EXPECT_EQ(3, table_model->size());
  EXPECT_EQ(L"11", table_model->row(0).cell(2).text());
  EXPECT_EQ(L"21", table_model->row(1).cell(2).text());
  EXPECT_EQ(L"", table_model->row(2).cell(2).text());
  for (auto row : table_model->rows()) {
    EXPECT_EQ(3, row->length()) << row->cell(0).text();
  }
}

TEST_F(TableModeTest, SetHeaderRow) {
  std::unique_ptr<ui::TableModel> table_model(new ui::TableModel());
  table_model->SetHeaderRow(L"foo\t\bar\tbaz");
  EXPECT_EQ(3, table_model->header_row()->length());
}

}  // namespace
