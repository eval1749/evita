// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_WINDOWS_SELECTION_H_
#define EVITA_DOM_WINDOWS_SELECTION_H_

#include "evita/dom/text/text_document.h"
#include "evita/dom/windows/text_document_window.h"
#include "evita/gc/member.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {

class TextDocument;
class TextDocumentWindow;

namespace bindings {
class SelectionClass;
}

//////////////////////////////////////////////////////////////////////
//
// Selection
//
class Selection : public v8_glue::Scriptable<Selection> {
  DECLARE_SCRIPTABLE_OBJECT(Selection);

 public:
  ~Selection() override;

  TextDocument* document() const { return document_; }

 protected:
  Selection(TextDocumentWindow* document_window, TextDocument* document);

  TextDocumentWindow* window() const { return document_window_; }

 private:
  friend class bindings::SelectionClass;

  gc::Member<TextDocument> document_;
  gc::Member<TextDocumentWindow> document_window_;

  DISALLOW_COPY_AND_ASSIGN(Selection);
};

}  // namespace dom

#endif  // EVITA_DOM_WINDOWS_SELECTION_H_
