// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/contents/tables/table_content.h"

#include "base/logging.h"
#include "evita/dom/contents/tables/table_row.h"
#include "evita/dom/script_controller.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/nullable.h"
#include "evita/v8_glue/function_template_builder.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {
namespace contents {

namespace {
//////////////////////////////////////////////////////////////////////
//
// TableContentClass
//
class TableContentClass :
    public v8_glue::DerivedWrapperInfo<TableContent, Content> {

  public: TableContentClass(const char* name)
      : BaseClass(name) {
  }
  public: ~TableContentClass() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &TableContentClass::NewTableContent);
  }

  private: static TableContent* NewTableContent(const base::string16& name) {
    return new TableContent(name);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("changeCount", &TableContent::change_count)
        .SetProperty("headerRow", &TableContent::header_row)
        .SetProperty("length", &TableContent::length)
        .SetMethod("addRow", &TableContent::AddRow)
        .SetMethod("removeRow", &TableContent::RemoveRow)
        .SetMethod("row", &TableContent::row);
  }

  DISALLOW_COPY_AND_ASSIGN(TableContentClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TableContent
//
DEFINE_SCRIPTABLE_OBJECT(TableContent, TableContentClass)

TableContent::TableContent(const base::string16& name)
    : ScriptableBase(name), change_count_(0), header_row_(new TableRow()) {
}

TableContent::~TableContent() {
}

TableRow* TableContent::row(size_t index) const {
  if (index >= rows_.size()) {
    ScriptController::instance()->ThrowError("Bad index.");
    return nullptr;
  }
  return rows_[index];
}

TableRow* TableContent::AddRow(TableRow* row) {
  rows_.push_back(row);
  row->set_table(this);
  ++change_count_;
  return row;
}

void TableContent::DidChangeRow(TableRow*) {
  ++change_count_;
}

void TableContent::RemoveRow(size_t index) {
  if (index >= rows_.size()) {
    ScriptController::instance()->ThrowError("Bad index");
    return;
  }
  auto present = rows_.begin() + index;
  (*present)->set_table(nullptr);
  rows_.erase(present);
  ++change_count_;
}

}  // namespace contents
}  // namespace dom
