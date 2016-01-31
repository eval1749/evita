// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_FORM_CONTROL_VISITOR_H_
#define EVITA_DOM_PUBLIC_FORM_CONTROL_VISITOR_H_

#include "base/macros.h"
#include "evita/dom/public/form_controls_forward.h"

namespace domapi {

//////////////////////////////////////////////////////////////////////
//
// FormControlVisitor
//
class FormControlVisitor {
 public:
  virtual ~FormControlVisitor();

#define V(Name) virtual void Visit##Name(Name* control) = 0;
  FOR_EACH_DOMAPI_FORM_CONTROL(V)
#undef V

  void Visit(const FormControl& control);

 protected:
  FormControlVisitor();

 private:
  DISALLOW_COPY_AND_ASSIGN(FormControlVisitor);
};

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_FORM_CONTROL_VISITOR_H_
