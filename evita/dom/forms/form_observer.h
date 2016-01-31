// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_FORMS_FORM_OBSERVER_H_
#define EVITA_DOM_FORMS_FORM_OBSERVER_H_

#include "base/macros.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// FormObserver
//
class FormObserver {
 public:
  virtual ~FormObserver();

  virtual void DidChangeForm() = 0;

 protected:
  FormObserver();

 private:
  DISALLOW_COPY_AND_ASSIGN(FormObserver);
};

}  // namespace dom

#endif  // EVITA_DOM_FORMS_FORM_OBSERVER_H_
