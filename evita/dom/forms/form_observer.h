// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_FORMS_FORM_OBSERVER_H_
#define EVITA_DOM_FORMS_FORM_OBSERVER_H_

namespace dom {

class FormObserver {
 public:
  virtual ~FormObserver();

  virtual void DidChangeForm() = 0;

 protected:
  FormObserver();
};

}  // namespace dom

#endif  // EVITA_DOM_FORMS_FORM_OBSERVER_H_
