// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/windows/table_selection.h"

#include "evita/dom/converter.h"
#include "evita/dom/script_host.h"
#include "evita/dom/windows/table_window.h"
#include "evita/dom/view_delegate.h"
#include "evita/v8_glue/constructor_template.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// TableSelection
//
TableSelection::TableSelection(TableWindow* table_window, Document* document)
    : ScriptableBase(table_window, document) {
}

TableSelection::~TableSelection() {
}

std::vector<int> TableSelection::GetRowStates(
    const std::vector<base::string16>& keys) const {
  return ScriptHost::instance()->view_delegate()->
      GetTableRowStates(window()->window_id(), keys);
}

}  // namespace dom
