// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_forms_form_h)
#define INCLUDE_evita_dom_forms_form_h

#include "evita/dom/events/view_event_target.h"

#include <unordered_map>
#include <vector>

#pragma warning(push)
#pragma warning(disable: 4625 4626)
#include "base/observer_list.h"
#pragma warning(pop)
#include "base/strings/string16.h"
#include "evita/dom/public/dialog_box_id.h"

namespace dom {

class FormControl;
class FormObserver;

class Form : public v8_glue::Scriptable<Form, ViewEventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(Form);

  private: std::unordered_map<int, FormControl*> controls_;
  private: const base::string16 name_;
  private: mutable ObserverList<FormObserver> observers_;

  public: Form(const base::string16& name);
  public: virtual ~Form();

  public: FormControl* control(int control_id) const;
  public: std::vector<FormControl*> controls() const;
  public: domapi::DialogBoxId dialog_box_id() const {
    return event_target_id();
  }
  public: const base::string16& name() const { return name_; }

  public: void AddFormControl(FormControl* control);
  public: void AddObserver(FormObserver* observer) const;
  public: void DidChangeFormControl(FormControl* control);
  public: void Realize();
  public: void RemoveObserver(FormObserver* observer) const;
  public: void Show();

  DISALLOW_COPY_AND_ASSIGN(Form);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_forms_form_h)
