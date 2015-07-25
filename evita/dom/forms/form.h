// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_forms_form_h)
#define INCLUDE_evita_dom_forms_form_h

#include "evita/dom/events/view_event_target.h"

#include <vector>

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

class Form : public v8_glue::Scriptable<Form, ViewEventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(Form);
  friend class bindings::FormClass;

  private: std::vector<FormControl*> controls_;
  private: gc::Member<FormControl> focus_control_;
  private: float height_;
  private: mutable base::ObserverList<FormObserver> observers_;
  private: base::string16 title_;
  private: float width_;

  private: Form();
  public: virtual ~Form();

  public: std::vector<FormControl*> controls() const;
  public: FormControl* focus_control() const { return focus_control_.get(); }
  private: void set_focus_control(
      v8_glue::Nullable<FormControl> new_focus_control);
  public: float height() const { return height_; }
  private: void set_height(float new_height);
  public: const base::string16& title() const { return title_; }
  private: void set_title(const base::string16& new_title);
  public: float width() const { return width_; }
  private: void set_width(float new_width);

  private: void AddFormControl(FormControl* control);
  public: void AddObserver(FormObserver* observer) const;
  public: void DidChangeFormControl(FormControl* control);
  public: void DidKillFocusFromFormControl(FormControl* control);
  public: void DidSetFocusToFormControl(FormControl* control);
  public: void RemoveObserver(FormObserver* observer) const;

  DISALLOW_COPY_AND_ASSIGN(Form);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_forms_form_h)
