// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_WINDOWS_TABLE_SELECTION_H_
#define EVITA_DOM_WINDOWS_TABLE_SELECTION_H_

#include <vector>

#include "base/strings/string16.h"
#include "evita/dom/windows/selection.h"
#include "evita/gc/member.h"
#include "evita/v8_glue/scriptable.h"

class Selection;

namespace dom {

class TextDocument;
class TextRange;
class TableWindow;

namespace bindings {
class TableSelectionClass;
}

//////////////////////////////////////////////////////////////////////
//
// TableSelection
//
class TableSelection final
    : public v8_glue::Scriptable<TableSelection, Selection> {
  DECLARE_SCRIPTABLE_OBJECT(TableSelection);

 public:
  TableSelection(TableWindow* table_window, TextDocument* document);
  ~TableSelection() final;

 private:
  friend class bindings::TableSelectionClass;

  std::vector<int> GetRowStates(const std::vector<base::string16>& keys) const;

  DISALLOW_COPY_AND_ASSIGN(TableSelection);
};

}  // namespace dom

#endif  // EVITA_DOM_WINDOWS_TABLE_SELECTION_H_
