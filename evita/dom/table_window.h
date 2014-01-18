// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_table_window_h)
#define INCLUDE_evita_dom_table_window_h

#include "evita/dom/document_window.h"

#include "evita/gc/member.h"

namespace dom {
class Document;

// The |TableWindow| is DOM world representative of UI world TextWidget, aka
// TextEditWindow.
class TableWindow : public v8_glue::Scriptable<TableWindow, DocumentWindow> {
  DECLARE_SCRIPTABLE_OBJECT(TableWindow);

  public: TableWindow(Document* document);
  public: virtual ~TableWindow();

  DISALLOW_COPY_AND_ASSIGN(TableWindow);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_table_window_h)
