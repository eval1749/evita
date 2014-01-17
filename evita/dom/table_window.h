// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_table_window_h)
#define INCLUDE_evita_dom_table_window_h

#include "evita/dom/window.h"

#include "evita/gc/member.h"

namespace dom {
class Document;

// The |TableWindow| is DOM world representative of UI world TextWidget, aka
// TextEditWindow.
class TableWindow : public v8_glue::Scriptable<TableWindow, Window> {
  DECLARE_SCRIPTABLE_OBJECT(TableWindow);

  private: gc::Member<Document> document_;

  public: TableWindow(Document* document);
  public: virtual ~TableWindow();

  public: Document* document() const { return document_; }

  DISALLOW_COPY_AND_ASSIGN(TableWindow);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_table_window_h)
