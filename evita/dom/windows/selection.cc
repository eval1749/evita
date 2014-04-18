// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/windows/selection.h"

#include "evita/dom/converter.h"
#include "evita/dom/text/document.h"
#include "evita/dom/windows/document_window.h"
#include "evita/dom/script_host.h"
#include "evita/v8_glue/constructor_template.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// Selection
//
Selection::Selection(DocumentWindow* document_window, Document* document)
    : document_(document), document_window_(document_window) {
}

Selection::~Selection() {
}

}  // namespace dom
