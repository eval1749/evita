// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/events/text_document_event.h"

#include "evita/dom/bindings/v8_glue_TextDocumentEventInit.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// TextDocumentEvent
//
TextDocumentEvent::TextDocumentEvent(const base::string16& type,
                                     const TextDocumentEventInit& init_dict)
    : ScriptableBase(type, init_dict), view_(init_dict.view()) {}

TextDocumentEvent::TextDocumentEvent(const base::string16& type)
    : TextDocumentEvent(type, TextDocumentEventInit()) {}

TextDocumentEvent::~TextDocumentEvent() {}

}  // namespace dom
