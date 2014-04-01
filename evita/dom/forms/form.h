// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_forms_form_h)
#define INCLUDE_evita_dom_forms_form_h

#include "evita/dom/events/view_event_target.h"

#include <vector>

#pragma warning(push)
#pragma warning(disable: 4625 4626)
#include "base/observer_list.h"
#pragma warning(pop)
#include "base/strings/string16.h"
#include "evita/gc/member.h"
#include "evita/v8_glue/nullable.h"

namespace dom {

class FormControl;
class FormObserver;

class Form : public v8_glue::Scriptable<Form, ViewEventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(Form);

  private: std::vector<FormControl*> controls_;
  private: gc::Member<FormControl> focus_control_;
  private: float height_;
  private: mutable ObserverList<FormObserver> observers_;
  private: base::string16 title_;
  private: float width_;

  public: Form();
  public: virtual ~Form();

  public: std::vector<FormControl*> controls() const;
  public: FormControl* focus_control() const { return focus_control_.get(); }
  public: void set_focus_control(
      v8_glue::Nullable<FormControl> new_focus_control);
  public: float height() const { return height_; }
  public: void set_height(float new_height);
  public: const base::string16& title() const { return title_; }
  public: void set_title(const base::string16& new_title);
  public: float width() const { return width_; }
  public: void set_width(float new_width);

  public: void AddFormControl(FormControl* control);
  public: void AddObserver(FormObserver* observer) const;
  public: void DidChangeFormControl(FormControl* control);
  public: void RemoveObserver(FormObserver* observer) const;

  DISALLOW_COPY_AND_ASSIGN(Form);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_forms_form_h)
