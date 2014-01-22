// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_ui_base_table_model_h)
#define INCLUDE_evita_ui_base_table_model_h

#include "base/strings/string16.h"

namespace ui {

class TableModel {
  public: TableModel();
  public: virtual ~TableModel();

  public: virtual int GetRowCount() const = 0;
  public: virtual int GetRowId(int index) const = 0;
  public: virtual base::string16 GetCellText(
    int row_id, int column_id) const = 0;

  DISALLOW_COPY_AND_ASSIGN(TableModel);
};

struct TableColumn {
  enum class Alignment {
    Left,
    Right,
    Center,
  };

  TableColumn(int column_id, Alignment alignment, float width);
  TableColumn();

  Alignment alignment;
  int column_id;
  base::string16 text;
  float width;
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_base_table_model_h)
