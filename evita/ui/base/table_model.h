// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_BASE_TABLE_MODEL_H_
#define EVITA_UI_BASE_TABLE_MODEL_H_

#include "base/macros.h"
#include "base/strings/string16.h"

namespace ui {

class TableModel {
 public:
  virtual ~TableModel();

  virtual int GetRowCount() const = 0;
  virtual int GetRowId(int index) const = 0;
  virtual base::string16 GetCellText(int row_id, int column_id) const = 0;

 protected:
  TableModel();

 private:
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

#endif  // EVITA_UI_BASE_TABLE_MODEL_H_
