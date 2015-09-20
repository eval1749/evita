// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/document_event.h"

#include "evita/bindings/v8_glue_DocumentEventInit.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// DocumentEvent
//
DocumentEvent::DocumentEvent(const base::string16& type,
                             const DocumentEventInit& init_dict)
    : ScriptableBase(type, init_dict), view_(init_dict.view()) {}

DocumentEvent::DocumentEvent(const base::string16& type)
    : DocumentEvent(type, DocumentEventInit()) {}

DocumentEvent::~DocumentEvent() {}

}  // namespace dom
