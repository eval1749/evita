// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_forms_label_control_h)
#define INCLUDE_evita_dom_forms_label_control_h

#include "evita/dom/forms/form_control.h"

namespace dom {

namespace bindings {
class LabelControlClass;
}

class LabelControl
    : public v8_glue::Scriptable<LabelControl, FormControl> {
  DECLARE_SCRIPTABLE_OBJECT(LabelControl);
  friend class bindings::LabelControlClass;

  private: base::string16 text_;

  private: LabelControl(const base::string16& text);
  public: virtual ~LabelControl();

  public: base::string16 text() const { return text_; }
  private: void set_text(const base::string16& text);

  DISALLOW_COPY_AND_ASSIGN(LabelControl);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_forms_label_control_h)
