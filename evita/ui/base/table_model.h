// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_ui_base_table_model_h)
#define INCLUDE_evita_ui_base_table_model_h

#include "base/strings/string16.h"

namespace ui {

class TableModel {
  public: TableModel();
  public: virtual ~TableModel();

  public: virtual int row_count() const = 0;
  public: virtual const base::string16& text(int row, int column_id) const = 0;
};

struct TableColumn {
  enum class Alignment {
    Left,
    Right,
    Center,
  };

  TableColumn(int id, Alignment alignment, int width);
  TableColumn();

  Alignment alignment;
  base::string16 text;
  int width;
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_base_table_model_h)
