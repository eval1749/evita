// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/text/modes/mode.h"

#include "evita/dom/text/document.h"
#include "evita/text/modes/mode.h"
#include "evita/v8_glue/converter.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// Mode
//
Mode::Mode(text::Mode* text_mode)
    : mode_(text_mode) {
}

Mode::~Mode() {
}

base::string16 Mode::name() const {
  return mode_->GetName();
}

void Mode::DoColor(int hint) {
  mode_->DoColor(hint);
}

}  // namespace dom
