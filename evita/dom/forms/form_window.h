// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_forms_form_window_h)
#define INCLUDE_evita_dom_forms_form_window_h

#include "evita/dom/windows/window.h"

#include "evita/gc/member.h"

namespace dom {

class Form;
class FormWindowInit;

namespace bindings {
class FormWindowClass;
}

//////////////////////////////////////////////////////////////////////
//
// FormWindow
//
class FormWindow final : public v8_glue::Scriptable<FormWindow, Window> {
  DECLARE_SCRIPTABLE_OBJECT(FormWindow);
  friend class bindings::FormWindowClass;

  private: gc::Member<Form> form_;

  private: FormWindow(Form* form, const FormWindowInit& init);
  private: FormWindow(Form* form);
  public: virtual ~FormWindow();

  public: Form* form() const { return form_.get(); }

  DISALLOW_COPY_AND_ASSIGN(FormWindow);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_forms_form_window_h)
