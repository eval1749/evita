// Copyright (c) 2013 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_WINDOWS_TEXT_DOCUMENT_WINDOW_H_
#define EVITA_DOM_WINDOWS_TEXT_DOCUMENT_WINDOW_H_

#include "evita/dom/windows/window.h"

#include "evita/gc/member.h"

namespace dom {

class TextDocument;
class Selection;

namespace bindings {
class TextDocumentWindowClass;
}

// The |TextDocumentWindow| is DOM world representative of UI world TextWidget,
// aka
// TextWindow.
class TextDocumentWindow
    : public v8_glue::Scriptable<TextDocumentWindow, Window> {
  DECLARE_SCRIPTABLE_OBJECT(TextDocumentWindow);

 public:
  ~TextDocumentWindow() override;

  TextDocument* document() const;

 protected:
  explicit TextDocumentWindow(Selection* selection);

  Selection* selection() const { return selection_; }

 private:
  friend class bindings::TextDocumentWindowClass;

  gc::Member<Selection> selection_;

  // Window
  void DidDestroyWindow() override;
  void DidRealizeWindow() override;

  DISALLOW_COPY_AND_ASSIGN(TextDocumentWindow);
};

}  // namespace dom

#endif  // EVITA_DOM_WINDOWS_TEXT_DOCUMENT_WINDOW_H_
