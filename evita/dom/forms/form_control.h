// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_FORMS_FORM_CONTROL_H_
#define EVITA_DOM_FORMS_FORM_CONTROL_H_

#include <memory>

#include "evita/dom/events/view_event_target.h"

#include "base/strings/string16.h"
#include "evita/dom/public/form_controls.h"
#include "evita/gc/member.h"

namespace dom {

class Form;
class FormPaintInfo;

namespace bindings {
class FormControlClass;
}

//////////////////////////////////////////////////////////////////////
//
// FormControl
//
class FormControl : public ginx::Scriptable<FormControl, ViewEventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(FormControl);

 public:
  ~FormControl() override;

  const domapi::IntRect& bounds() const { return bounds_; }

  // Expose |clientLeft|, |clientTop|, |clientWidth| and |clientHeight| for
  // layout management in |views::FormWindow|.
  int client_height() const { return bounds_.height(); }
  int client_left() const { return bounds_.x(); }
  int client_top() const { return bounds_.y(); }
  int client_width() const { return bounds_.width(); }

  // Disabled control can't get focus.
  // Expose |disabled| for |views::FormWindow|.
  bool disabled() const { return disabled_; }

  // TODO(eval1749): We should move |FormControl.prototype.name| to JavaScript.
  // Exposed for |RadioButton|.
  const base::string16& name() const { return name_; }

  virtual std::unique_ptr<domapi::FormControl> Paint(
      const FormPaintInfo& paint_info) const = 0;

 protected:
  explicit FormControl(const base::string16& name);
  FormControl();

  // Associated form.
  Form* form() const { return form_.get(); }

  // True if this |FormControl| is handling form event.
  bool handling_form_event() const { return handling_form_event_; }

  domapi::FormControl::State ComputeState(
      const FormPaintInfo& paint_info) const;
  void NotifyControlChange();

 private:
  friend class bindings::FormControlClass;
  friend class Form;  // for updating form_
  friend class HandlingFormEventScope;

  // bindings
  void set_client_height(float new_client_hieght);
  void set_client_left(float new_client_left);
  void set_client_top(float new_client_top);
  void set_client_width(float new_client_width);
  void set_disabled(bool new_disabled);

  // dom::EventTarget
  EventPath BuildEventPath() const final;

  domapi::IntRect bounds_;
  bool disabled_;
  gc::Member<Form> form_;
  bool handling_form_event_;
  base::string16 name_;

  DISALLOW_COPY_AND_ASSIGN(FormControl);
};

}  // namespace dom

#endif  // EVITA_DOM_FORMS_FORM_CONTROL_H_
