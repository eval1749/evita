// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_WINDOWS_TABLE_WINDOW_H_
#define EVITA_DOM_WINDOWS_TABLE_WINDOW_H_

#include "evita/dom/windows/text_document_window.h"

#include "evita/gc/member.h"

namespace dom {

class TextDocument;

namespace bindings {
class TableWindowClass;
}

// The |TableWindow| is DOM world representative of UI world TextWidget, aka
// TextWindow.
class TableWindow final
    : public v8_glue::Scriptable<TableWindow, TextDocumentWindow> {
  DECLARE_SCRIPTABLE_OBJECT(TableWindow);

 public:
  ~TableWindow() final;

 private:
  friend class bindings::TableWindowClass;

  explicit TableWindow(TextDocument* document);

  DISALLOW_COPY_AND_ASSIGN(TableWindow);
};

}  // namespace dom

#endif  // EVITA_DOM_WINDOWS_TABLE_WINDOW_H_
