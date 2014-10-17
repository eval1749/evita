// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_forms_form_control_h)
#define INCLUDE_evita_dom_forms_form_control_h

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
  friend class Form; // for updating form_
  friend class bindings::FormControlClass;

  protected: class HandlingFormEventScope {
    private: FormControl* control_;
    public: HandlingFormEventScope(FormControl* control);
    public: ~HandlingFormEventScope();
  };
  friend class HandlingFormEventScope;

  private: gfx::RectF bounds_;
  private: bool disabled_;
  private: gc::Member<Form> form_;
  private: bool handling_form_event_;
  private: base::string16 name_;

  protected: explicit FormControl(const base::string16& name);
  protected: FormControl();
  protected: virtual ~FormControl();

  // Expose |clientLeft|, |clientTop|, |clientWidth| and |clientHeight| for
  // layout management in |views::FormWindow|.
  public: float client_height() const { return bounds_.height(); }
  private: void set_client_height(float new_client_hieght);
  public: float client_left() const { return bounds_.left; }
  private: void set_client_left(float new_client_left);
  public: float client_top() const { return bounds_.top; }
  private: void set_client_top(float new_client_top);
  public: float client_width() const { return bounds_.width(); }
  private: void set_client_width(float new_client_width);

  // Disabled control can't get focus.
  // Expose |disabled| for |views::FormWindow|.
  public: bool disabled() const { return disabled_; }
  private: void set_disabled(bool new_disabled);

  // Associated form.
  protected: Form* form() const { return form_.get(); }

  // True if this |FormControl| is handling form event.
  protected: bool handling_form_event() const { return handling_form_event_; }

  // Exposed for |RadioButton|.
  public: const base::string16& name() const { return name_; }

  public: void DidKillFocus();
  public: void DidSetFocus();
  public: void DispatchChangeEvent();
  protected: void NotifyControlChange();

  // dom::EventTarget
  private: virtual EventPath BuildEventPath() const override;

  DISALLOW_COPY_AND_ASSIGN(FormControl);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_forms_form_control_h)
