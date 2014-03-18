// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/windows/document_window.h"

#include "base/bind.h"
#include "evita/dom/events/document_event.h"
#include "evita/dom/events/document_event_init.h"
#include "evita/dom/public/tab_data.h"
#include "evita/dom/text/document.h"
#include "evita/dom/script_controller.h"
#include "evita/dom/view_delegate.h"
#include "evita/dom/windows/selection.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// DocumentWindowClass
//
class DocumentWindowClass :
    public v8_glue::DerivedWrapperInfo<DocumentWindow, Window> {

  public: DocumentWindowClass(const char* name)
      : BaseClass(name) {
  }
  public: ~DocumentWindowClass() = default;

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("document", &DocumentWindow::document)
        .SetProperty("selection", &DocumentWindow::selection);
  }

  DISALLOW_COPY_AND_ASSIGN(DocumentWindowClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// DocumentWindow
//
DEFINE_SCRIPTABLE_OBJECT(DocumentWindow, DocumentWindowClass);

DocumentWindow::DocumentWindow(Selection* selection)
    : selection_(selection) {
  // TODO(yosi) We should not set |TabData| in |dom::DocumentWindow|
  // constructor, but befor |dom::Window::Realize| to make sure tab strip
  // to have tab data.
  auto const document = selection->document();
  domapi::TabData tab_data;
  tab_data.title = document->name();
  tab_data.tooltip = document->name();
  tab_data.state = domapi::TabData::State::Loading;
  // TODO(yosi) We should not use magic value -2 for tab_data.icon.
  tab_data.icon = -2;
  ScriptController::instance()->view_delegate()->SetTabData(
      window_id(), tab_data);
}

DocumentWindow::~DocumentWindow() {
}

Document* DocumentWindow::document() const {
  return selection_->document();
}

// Window
void DocumentWindow::DidDestroyWindow() {
  Window::DidDestroyWindow();
  selection_->document()->ScheduleDispatchEvent(
      new DocumentEvent(L"detach", DocumentEventInit(this)));
}

void DocumentWindow::DidRealizeWindow() {
  Window::DidRealizeWindow();
  selection_->document()->ScheduleDispatchEvent(
      new DocumentEvent(L"attach", DocumentEventInit(this)));
}

}  // namespace dom
