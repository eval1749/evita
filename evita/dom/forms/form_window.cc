// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/forms/form_window.h"

#include "evita/bindings/v8_glue_FormWindowInit.h"
#include "evita/dom/public/view_delegate.h"
#include "evita/dom/script_host.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// FormWindow
//
FormWindow::FormWindow(Form* form, const FormWindowInit& init) : form_(form) {
  domapi::PopupWindowInit window_init;
  window_init.offset_x = init.offset_x();
  window_init.offset_y = init.offset_y();
  window_init.owner_id =
      init.owner() ? init.owner()->window_id() : domapi::kInvalidWindowId;
  ScriptHost::instance()->view_delegate()->CreateFormWindow(window_id(), form,
                                                            window_init);
}

FormWindow::FormWindow(Form* form) : FormWindow(form, FormWindowInit()) {}

FormWindow::~FormWindow() {}

}  // namespace dom
