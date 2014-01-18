// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_table_selection_h)
#define INCLUDE_evita_dom_table_selection_h

#include <vector>

#include "base/strings/string16.h"
#include "evita/dom/selection.h"
#include "evita/gc/member.h"
#include "evita/v8_glue/scriptable.h"

class Selection;

namespace dom {

class Document;
class Range;
class TableWindow;

//////////////////////////////////////////////////////////////////////
//
// TableSelection
//
class TableSelection : public v8_glue::Scriptable<TableSelection, Selection> {
  DECLARE_SCRIPTABLE_OBJECT(TableSelection);

  public: TableSelection(TableWindow* table_window, Document* document);
  public: virtual ~TableSelection();

  public: std::vector<int> GetRowStates(
      const std::vector<base::string16>& keys) const;

  DISALLOW_COPY_AND_ASSIGN(TableSelection);
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_table_selection_h)
