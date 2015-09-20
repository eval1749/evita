// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_FORMS_FORM_CONTROL_H_
#define EVITA_DOM_FORMS_FORM_CONTROL_H_

#include "evita/dom/events/view_event_target.h"

#include "base/strings/string16.h"
#include "evita/gc/member.h"
#include "evita/gfx/rect_f.h"

namespace dom {

class Form;

namespace bindings {
class FormControlClass;
}

class FormControl : public v8_glue::Scriptable<FormControl, ViewEventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(FormControl);

 public:
  ~FormControl() override;

  // Expose |clientLeft|, |clientTop|, |clientWidth| and |clientHeight| for
  // layout management in |views::FormWindow|.
  float client_height() const { return bounds_.height(); }
  float client_left() const { return bounds_.left; }
  float client_top() const { return bounds_.top; }
  float client_width() const { return bounds_.width(); }

  // Disabled control can't get focus.
  // Expose |disabled| for |views::FormWindow|.
  bool disabled() const { return disabled_; }

  // Exposed for |RadioButton|.
  const base::string16& name() const { return name_; }

  void DidKillFocus();
  void DidSetFocus();
  void DispatchChangeEvent();

 protected:
  class HandlingFormEventScope final {
   public:
    explicit HandlingFormEventScope(FormControl* control);
    ~HandlingFormEventScope();

   private:
    FormControl* control_;

    DISALLOW_COPY_AND_ASSIGN(HandlingFormEventScope);
  };

  explicit FormControl(const base::string16& name);
  FormControl();

  // Associated form.
  Form* form() const { return form_.get(); }

  // True if this |FormControl| is handling form event.
  bool handling_form_event() const { return handling_form_event_; }

  void NotifyControlChange();

 private:
  friend class bindings::FormControlClass;
  friend class Form;  // for updating form_
  friend class HandlingFormEventScope;

  void set_client_height(float new_client_hieght);
  void set_client_left(float new_client_left);
  void set_client_top(float new_client_top);
  void set_client_width(float new_client_width);
  void set_disabled(bool new_disabled);

  // dom::EventTarget
  EventPath BuildEventPath() const override;

  gfx::RectF bounds_;
  bool disabled_;
  gc::Member<Form> form_;
  bool handling_form_event_;
  base::string16 name_;

  DISALLOW_COPY_AND_ASSIGN(FormControl);
};

}  // namespace dom

#endif  // EVITA_DOM_FORMS_FORM_CONTROL_H_
