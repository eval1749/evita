// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_forms_form_control_h)
#define INCLUDE_evita_dom_forms_form_control_h

#include "evita/dom/events/view_event_target.h"

#include "base/strings/string16.h"
#include "evita/gc/member.h"

namespace dom {

typedef int FormResourceId;
const FormResourceId kInvalidFormResourceId = -1;

class Form;

class FormControl : public v8_glue::Scriptable<FormControl, ViewEventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(FormControl);
  friend class Form; // for updating form_

  protected: class HandlingFormEventScope {
    private: FormControl* control_;
    public: HandlingFormEventScope(FormControl* control);
    public: ~HandlingFormEventScope();
  };
  friend class HandlingFormEventScope;

  private: FormResourceId control_id_;
  private: bool disabled_;
  private: gc::Member<Form> form_;
  private: bool handling_form_event_;
  private: base::string16 name_;

  protected: FormControl(int control_id, const base::string16& name);
  protected: explicit FormControl(int control_id);
  protected: FormControl();
  public: virtual ~FormControl();

  public: FormResourceId control_id() const { return control_id_; }
  public: bool disabled() const { return disabled_; }
  public: void set_disabled(bool new_disabled);
  public: Form* form() const { return form_.get(); }
  protected: bool handling_form_event() const { return handling_form_event_; }
  public: const base::string16& name() const { return name_; }

  public: void DispatchChangeEvent();

  DISALLOW_COPY_AND_ASSIGN(FormControl);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_forms_form_control_h)
