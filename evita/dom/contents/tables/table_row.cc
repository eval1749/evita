// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/contents/tables/table_row.h"

#include <algorithm>

#include "evita/dom/contents/tables/table_cell.h"
#include "evita/dom/contents/tables/table_content.h"
#include "evita/dom/script_controller.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {
namespace contents {

namespace {
//////////////////////////////////////////////////////////////////////
//
// TableRowClass
//
class TableRowClass : public v8_glue::WrapperInfo {
  public: TableRowClass(const char* name)
      : v8_glue::WrapperInfo(name) {
  }
  public: ~TableRowClass() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &TableRowClass::NewTableRow);
  }

  private: static TableRow* NewTableRow() {
    return new TableRow();
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("length", &TableRow::length)
        .SetMethod("addCell_", &TableRow::AddCell)
        .SetMethod("cell", &TableRow::cell)
        .SetMethod("removeCell", &TableRow::RemoveCell)
        .SetMethod("setCell", &TableRow::set_cell);
  }

  DISALLOW_COPY_AND_ASSIGN(TableRowClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TableRow
//
DEFINE_SCRIPTABLE_OBJECT(TableRow, TableRowClass)

TableRow::TableRow() {
}

TableRow::~TableRow() {
}

TableCell* TableRow::cell(size_t index) const {
  if (static_cast<size_t>(index) >= cells_.size()) {
    ScriptController::instance()->ThrowError("Invalid idnex.");
    return nullptr;
  }
  return cells_[index];
}

void TableRow::set_cell(size_t index, TableCell* cell) {
  if (static_cast<size_t>(index) >= cells_.size()) {
    ScriptController::instance()->ThrowError("Invalid idnex.");
    return;
  }
  if (cell->row()) {
    cell->row()->RemoveCell(cell);
  }
  cells_[index] = cell;
  cell->set_row(this);
  if (table_)
    table_->DidChangeRow(this);
}

void TableRow::AddCell(TableCell* cell) {
  if (cell->row()) {
    cell->row()->RemoveCell(cell);
  }
  cells_.push_back(cell);
  cell->set_row(this);
  if (table_)
    table_->DidChangeRow(this);
}

void TableRow::DidChangeCell(TableCell*) {
  if (!table_)
    return;
  table_->DidChangeRow(this);
}

void TableRow::RemoveCell(TableCell* cell) {
  auto present = std::find(cells_.begin(), cells_.end(), cell);
  if (cell->row() != this || present == cells_.end()) {
    ScriptController::instance()->ThrowError("Invalid cell.");
    return;
  }
  cells_.erase(present);
  cell->set_row(nullptr);
  if (table_)
    table_->DidChangeRow(this);
}

}  // namespace contents
}  // namespace dom
