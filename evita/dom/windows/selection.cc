// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/windows/selection.h"

#include "evita/dom/converter.h"
#include "evita/dom/text/document.h"
#include "evita/dom/windows/document_window.h"
#include "evita/dom/script_controller.h"
#include "evita/v8_glue/constructor_template.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// SelectionClass
//
class SelectionClass : public v8_glue::WrapperInfo {
  public: SelectionClass(const char* name)
      : v8_glue::WrapperInfo(name) {
  }
  public: ~SelectionClass() = default;

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("document", &Selection::document)
        .SetProperty("window", &Selection::window);
  }

  DISALLOW_COPY_AND_ASSIGN(SelectionClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Selection
//
DEFINE_SCRIPTABLE_OBJECT(Selection, SelectionClass);

Selection::Selection(DocumentWindow* document_window, Document* document)
    : document_(document), document_window_(document_window) {
}

Selection::~Selection() {
}

}  // namespace dom
