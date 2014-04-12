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

class FormControl : public v8_glue::Scriptable<FormControl, ViewEventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(FormControl);
  friend class Form; // for updating form_

  protected: class HandlingFormEventScope {
    private: FormControl* control_;
    public: HandlingFormEventScope(FormControl* control);
    public: ~HandlingFormEventScope();
  };
  friend class HandlingFormEventScope;

  private: bool disabled_;
  private: gc::Member<Form> form_;
  private: bool handling_form_event_;
  private: base::string16 name_;
  private: gfx::RectF rect_;

  protected: explicit FormControl(const base::string16& name);
  protected: FormControl();
  public: virtual ~FormControl();

  public: float client_height() const { return rect_.height(); }
  public: void set_client_height(float new_client_hieght);
  public: float client_left() const { return rect_.left; }
  public: void set_client_left(float new_client_left);
  public: float client_top() const { return rect_.top; }
  public: void set_client_top(float new_client_top);
  public: float client_width() const { return rect_.width(); }
  public: void set_client_width(float new_client_width);
  public: bool disabled() const { return disabled_; }
  public: void set_disabled(bool new_disabled);
  public: Form* form() const { return form_.get(); }
  protected: bool handling_form_event() const { return handling_form_event_; }
  public: const base::string16& name() const { return name_; }

  public: void DidSetFocus();
  public: void DispatchChangeEvent();
  protected: void NotifyControlChange();
  public: void ReleaseCapture();
  public: void SetCapture();

  // dom::EventTarget
  private: virtual EventPath BuildEventPath() const override;

  DISALLOW_COPY_AND_ASSIGN(FormControl);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_forms_form_control_h)
