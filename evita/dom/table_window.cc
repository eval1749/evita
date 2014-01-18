// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/table_window.h"

#include "evita/dom/document.h"
#include "evita/dom/range.h"
#include "evita/dom/text_selection.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// TableWindowClass
//
class TableWindowClass :
    public v8_glue::DerivedWrapperInfo<TableWindow, Window> {

  public: TableWindowClass(const char* name)
      : BaseClass(name) {
  }
  public: ~TableWindowClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &TableWindowClass::NewTableWindow);
  }

  private: static TableWindow* NewTableWindow(Document* document) {
    return new TableWindow(document);
  }
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TableWindow
//
DEFINE_SCRIPTABLE_OBJECT(TableWindow, TableWindowClass);

TableWindow::TableWindow(Document* document)
    // TODO(yosi) Until we have |TableSelection|, we use |Seleciton|.
    : ScriptableBase(new TextSelection(reinterpret_cast<TextWindow*>(this),
                                       new Range(document, 0, 0))) {
  ScriptController::instance()->view_delegate()->CreateTableWindow(
      window_id(), document);
}

TableWindow::~TableWindow() {
}

}  // namespace dom
