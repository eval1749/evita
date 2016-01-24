// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_FORMS_FORM_WINDOW_H_
#define EVITA_DOM_FORMS_FORM_WINDOW_H_

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

 public:
  ~FormWindow() final;

  Form* form() const { return form_.get(); }

 private:
  friend class bindings::FormWindowClass;

  // bindings
  FormWindow(ScriptHost* script_host, Form* form, const FormWindowInit& init);
  FormWindow(ScriptHost* script_host, Form* form);

  gc::Member<Form> form_;

  DISALLOW_COPY_AND_ASSIGN(FormWindow);
};

}  // namespace dom

#endif  // EVITA_DOM_FORMS_FORM_WINDOW_H_
