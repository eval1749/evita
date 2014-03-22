// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_forms_form_window_h)
#define INCLUDE_evita_dom_forms_form_window_h

#include "evita/dom/windows/window.h"

#include "evita/gc/member.h"

namespace dom {

class Form;

//////////////////////////////////////////////////////////////////////
//
// FormWindow
//
class FormWindow : public v8_glue::Scriptable<FormWindow, Window> {
  DECLARE_SCRIPTABLE_OBJECT(FormWindow);

  private: gc::Member<dom::Form> form_;

  public: FormWindow(Form* form);
  public: virtual ~FormWindow();

  public: Form* form() const { return form_.get(); }

  DISALLOW_COPY_AND_ASSIGN(FormWindow);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_forms_form_window_h)
