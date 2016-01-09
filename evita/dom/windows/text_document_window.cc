// Copyright (c) 2013 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/windows/text_document_window.h"

#include "base/bind.h"
#include "evita/bindings/v8_glue_TextDocumentEventInit.h"
#include "evita/dom/events/text_document_event.h"
#include "evita/dom/public/tab_data.h"
#include "evita/dom/text/text_document.h"
#include "evita/dom/windows/selection.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// TextDocumentWindow
//
TextDocumentWindow::TextDocumentWindow(Selection* selection)
    : selection_(selection) {}

TextDocumentWindow::~TextDocumentWindow() {}

TextDocument* TextDocumentWindow::document() const {
  return selection_->document();
}

// Window
void TextDocumentWindow::DidDestroyWindow() {
  // TODO(eval1749): We should dispatch "detach" event in JavaScript rather than
  // in C++;
  Window::DidDestroyWindow();
  TextDocumentEventInit init;
  init.set_bubbles(true);
  init.set_view(this);
  selection_->document()->ScheduleDispatchEventDeprecated(
      new TextDocumentEvent(L"detach", init));
}

void TextDocumentWindow::DidRealizeWindow() {
  // TODO(eval1749): We should dispatch "attach" event in JavaScript rather than
  // in C++;
  Window::DidRealizeWindow();
  TextDocumentEventInit init;
  init.set_bubbles(true);
  init.set_view(this);
  selection_->document()->ScheduleDispatchEventDeprecated(
      new TextDocumentEvent(L"attach", init));
}

}  // namespace dom
