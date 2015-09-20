// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_WINDOWS_DOCUMENT_WINDOW_H_
#define EVITA_DOM_WINDOWS_DOCUMENT_WINDOW_H_

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

 public:
  ~DocumentWindow() override;

  Document* document() const;

 protected:
  explicit DocumentWindow(Selection* selection);

  Selection* selection() const { return selection_; }

 private:
  friend class bindings::DocumentWindowClass;

  gc::Member<Selection> selection_;

  // Window
  void DidDestroyWindow() override;
  void DidRealizeWindow() override;

  DISALLOW_COPY_AND_ASSIGN(DocumentWindow);
};

}  // namespace dom

#endif  // EVITA_DOM_WINDOWS_DOCUMENT_WINDOW_H_
