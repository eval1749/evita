// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_FORMS_FORM_H_
#define EVITA_DOM_FORMS_FORM_H_

#include <vector>

#include "evita/dom/events/view_event_target.h"

#include "base/observer_list.h"
#include "base/strings/string16.h"
#include "evita/gc/member.h"
#include "evita/v8_glue/nullable.h"

namespace dom {

class FormControl;
class FormObserver;

namespace bindings {
class FormClass;
}

class Form final : public v8_glue::Scriptable<Form, ViewEventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(Form);

 public:
  ~Form() final;

  std::vector<FormControl*> controls() const;
  FormControl* focus_control() const { return focus_control_.get(); }
  float height() const { return height_; }
  const base::string16& title() const { return title_; }
  float width() const { return width_; }

  void AddObserver(FormObserver* observer) const;
  void DidChangeFormControl(FormControl* control);
  void DidKillFocusFromFormControl(FormControl* control);
  void DidSetFocusToFormControl(FormControl* control);
  void RemoveObserver(FormObserver* observer) const;

 private:
  friend class bindings::FormClass;

  Form();

  void set_focus_control(v8_glue::Nullable<FormControl> new_focus_control);
  void set_height(float new_height);
  void set_title(const base::string16& new_title);
  void set_width(float new_width);

  void AddFormControl(FormControl* control);

  std::vector<FormControl*> controls_;
  gc::Member<FormControl> focus_control_;
  float height_;
  mutable base::ObserverList<FormObserver> observers_;
  base::string16 title_;
  float width_;

  DISALLOW_COPY_AND_ASSIGN(Form);
};

}  // namespace dom

#endif  // EVITA_DOM_FORMS_FORM_H_
