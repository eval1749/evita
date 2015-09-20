// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_FORMS_LABEL_CONTROL_H_
#define EVITA_DOM_FORMS_LABEL_CONTROL_H_

#include "evita/dom/forms/form_control.h"

namespace dom {

namespace bindings {
class LabelControlClass;
}

class LabelControl final
    : public v8_glue::Scriptable<LabelControl, FormControl> {
  DECLARE_SCRIPTABLE_OBJECT(LabelControl);

 public:
  ~LabelControl() final;

  base::string16 text() const { return text_; }

 private:
  friend class bindings::LabelControlClass;

  explicit LabelControl(const base::string16& text);

  void set_text(const base::string16& text);

  base::string16 text_;

  DISALLOW_COPY_AND_ASSIGN(LabelControl);
};

}  // namespace dom

#endif  // EVITA_DOM_FORMS_LABEL_CONTROL_H_
