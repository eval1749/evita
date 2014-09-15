// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_document_window_h)
#define INCLUDE_evita_dom_document_window_h

#include "evita/dom/windows/window.h"

#include "evita/gc/member.h"

namespace dom {

class Document;
class Selection;

namespace bindings {
class DocumentWindowClass;
}

// The |DocumentWindow| is DOM world representative of UI world TextWidget, aka
// TextWindow.
class DocumentWindow : public v8_glue::Scriptable<DocumentWindow, Window> {
  DECLARE_SCRIPTABLE_OBJECT(DocumentWindow);
  friend class bindings::DocumentWindowClass;

  private: gc::Member<Selection> selection_;

  protected: DocumentWindow(Selection* selection);
  protected: virtual ~DocumentWindow();

  public: Document* document() const;
  protected: Selection* selection() const { return selection_; }

  // Window
  private: virtual void DidDestroyWindow() override;
  private: virtual void DidRealizeWindow() override;

  DISALLOW_COPY_AND_ASSIGN(DocumentWindow);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_document_window_h)
