// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_document_window_h)
#define INCLUDE_evita_dom_document_window_h

#include "evita/dom/windows/window.h"

#include "evita/gc/member.h"

namespace dom {
class Document;
class Selection;

// The |DocumentWindow| is DOM world representative of UI world TextWidget, aka
// TextEditWindow.
class DocumentWindow : public v8_glue::Scriptable<DocumentWindow, Window> {
  DECLARE_SCRIPTABLE_OBJECT(DocumentWindow);

  private: gc::Member<Selection> selection_;

  public: DocumentWindow(Selection* selection);
  public: virtual ~DocumentWindow();

  public: Document* document() const;
  public: Selection* selection() const { return selection_; }

  DISALLOW_COPY_AND_ASSIGN(DocumentWindow);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_document_window_h)
