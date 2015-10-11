// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/windows/document_window.h"

#include "base/bind.h"
#include "evita/bindings/v8_glue_DocumentEventInit.h"
#include "evita/dom/events/document_event.h"
#include "evita/dom/public/tab_data.h"
#include "evita/dom/text/document.h"
#include "evita/dom/windows/selection.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// DocumentWindow
//
DocumentWindow::DocumentWindow(Selection* selection) : selection_(selection) {}

DocumentWindow::~DocumentWindow() {}

Document* DocumentWindow::document() const {
  return selection_->document();
}

// Window
void DocumentWindow::DidDestroyWindow() {
  // TODO(eval1749): We should dispatch "detach" event in JavaScript rather than
  // in C++;
  Window::DidDestroyWindow();
  DocumentEventInit init;
  init.set_bubbles(true);
  init.set_view(this);
  selection_->document()->ScheduleDispatchEvent(
      new DocumentEvent(L"detach", init));
}

void DocumentWindow::DidRealizeWindow() {
  // TODO(eval1749): We should dispatch "attach" event in JavaScript rather than
  // in C++;
  Window::DidRealizeWindow();
  DocumentEventInit init;
  init.set_bubbles(true);
  init.set_view(this);
  selection_->document()->ScheduleDispatchEvent(
      new DocumentEvent(L"attach", init));
}

}  // namespace dom
