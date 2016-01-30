// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_FORMS_FORM_H_
#define EVITA_DOM_FORMS_FORM_H_

#include <vector>

#include "evita/dom/events/view_event_target.h"

#include "base/observer_list.h"
#include "base/strings/string16.h"
#include "evita/gc/member.h"
#include "evita/geometry/int_rect.h"
#include "evita/v8_glue/nullable.h"

namespace dom {

class ExceptionState;
class FormControl;
class FormObserver;

namespace bindings {
class FormClass;
}

class Form final : public v8_glue::Scriptable<Form, ViewEventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(Form);

 public:
  using IntPoint = evita::IntPoint;
  using IntRect = evita::IntRect;
  using IntSize = evita::IntSize;

  ~Form() final;

  std::vector<FormControl*> controls() const;
  FormControl* focus_control() const { return focus_control_.get(); }
  int height() const { return bounds_.height(); }
  const base::string16& title() const { return title_; }
  int width() const { return bounds_.width(); }

  void AddObserver(FormObserver* observer) const;
  void DidChangeFormControl(FormControl* control);
  void DidKillFocusFromFormControl(FormControl* control);
  void DidSetFocusToFormControl(FormControl* control);
  void RemoveObserver(FormObserver* observer) const;

 private:
  friend class bindings::FormClass;

  Form();

  void set_focus_control(v8_glue::Nullable<FormControl> new_focus_control);
  void set_height(int new_height);
  void set_title(const base::string16& new_title);
  void set_width(int new_width);

  void AddFormControl(FormControl* control, ExceptionState* exception_state);

  IntRect bounds_;
  std::vector<FormControl*> controls_;
  gc::Member<FormControl> focus_control_;
  mutable base::ObserverList<FormObserver> observers_;
  base::string16 title_;

  DISALLOW_COPY_AND_ASSIGN(Form);
};

}  // namespace dom

#endif  // EVITA_DOM_FORMS_FORM_H_
