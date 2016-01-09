// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/windows/selection.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// Selection
//
Selection::Selection(TextDocumentWindow* document_window,
                     TextDocument* document)
    : document_(document), document_window_(document_window) {}

Selection::~Selection() {}

}  // namespace dom
