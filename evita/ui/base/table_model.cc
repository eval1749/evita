// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/ui/base/table_model.h"

namespace ui {

TableModel::TableModel() {
}

TableModel::~TableModel() {
}

TableColumn::TableColumn(int column_id,  Alignment alignment, float width)
    : alignment(alignment), column_id(column_id), width(width) {
}

TableColumn::TableColumn()
    : alignment(Alignment::Left), column_id(0), width(0.0f) {
}

}  // namespace ui
