// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/form_window.h"

#include "evita/bindings/FormWindowInit.h"
#include "evita/dom/script_host.h"
#include "evita/dom/view_delegate.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// FormWindow
//
FormWindow::FormWindow(Form* form, const FormWindowInit&) : form_(form) {
  ScriptHost::instance()->view_delegate()->CreateFormWindow(
      window_id(), form);
}

FormWindow::FormWindow(Form* form) : FormWindow(form, FormWindowInit()) {
}

FormWindow::~FormWindow() {
}

FormWindow* FormWindow::NewFormWindow(
    Form* form, v8_glue::Optional<FormWindowInit> opt_init) {
  if (opt_init.is_supplied)
    return new FormWindow(form, opt_init.value);
  return new FormWindow(form);
}

}  // namespace dom
