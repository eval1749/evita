// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/contents/tables/table_cell.h"

#include "evita/dom/contents/tables/table_row.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {
namespace contents {
namespace {
//////////////////////////////////////////////////////////////////////
//
// TableCellClass
//
class TableCellClass : public v8_glue::WrapperInfo {
  public: TableCellClass(const char* name)
      : v8_glue::WrapperInfo(name) {
  }
  public: ~TableCellClass() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &TableCellClass::NewTableCell);
  }

  private: static TableCell* NewTableCell(const base::string16& text) {
    return new TableCell(text);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("text", &TableCell::text, &TableCell::set_text);
  }

  DISALLOW_COPY_AND_ASSIGN(TableCellClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TableCell
//
DEFINE_SCRIPTABLE_OBJECT(TableCell, TableCellClass)

TableCell::TableCell(const base::string16& text) : text_(text) {
}

TableCell::~TableCell() {
}

void TableCell::set_text(const base::string16& text) {
  if (text_ != text) {
    if (row_)
      row_->DidChangeCell(this);
    return;
  }
  text_ = text;
}

}  // namespace contents
}  // namespace dom
