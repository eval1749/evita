// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/windows/table_window.h"

#include "evita/dom/script_host.h"
#include "evita/dom/view_delegate.h"
#include "evita/dom/windows/table_selection.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// TableWindow
//
TableWindow::TableWindow(Document* document)
    : ScriptableBase(new TableSelection(this, document)) {
  ScriptHost::instance()->view_delegate()->CreateTableWindow(
      window_id(), document);
}

TableWindow::~TableWindow() {
}

}  // namespace dom
