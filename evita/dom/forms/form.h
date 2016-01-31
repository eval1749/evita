// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_FORMS_FORM_H_
#define EVITA_DOM_FORMS_FORM_H_

#include <memory>
#include <vector>

#include "evita/dom/events/view_event_target.h"

#include "base/observer_list.h"
#include "base/strings/string16.h"
#include "evita/dom/public/geometry.h"
#include "evita/gc/member.h"
#include "evita/ui/system_metrics_observer.h"
#include "evita/v8_glue/nullable.h"

namespace domapi {
class Form;
}

namespace visuals {
class TextFormat;
}

namespace dom {

class ExceptionState;
class FormControl;
class FormObserver;
class FormPaintInfo;

namespace bindings {
class FormClass;
}

//////////////////////////////////////////////////////////////////////
//
// Form
//
class Form final : public v8_glue::Scriptable<Form, ViewEventTarget>,
                   public ui::SystemMetricsObserver {
  DECLARE_SCRIPTABLE_OBJECT(Form);

 public:
  ~Form() final;

  const domapi::IntRect& bounds() const { return bounds_; }
  std::vector<FormControl*> controls() const;
  FormControl* focus_control() const { return focus_control_.get(); }
  int height() const { return bounds_.height(); }
  const base::string16& title() const { return title_; }
  int width() const { return bounds_.width(); }

  void AddObserver(FormObserver* observer) const;
  const visuals::TextFormat& GetTextFormat() const;
  void DidChangeFormControl(FormControl* control);
  std::unique_ptr<domapi::Form> Paint(const FormPaintInfo& paint_info) const;
  void RemoveObserver(FormObserver* observer) const;

 private:
  friend class bindings::FormClass;

  Form();

  void NotifyChangeForm();

  // bindings
  void set_focus_control(v8_glue::Nullable<FormControl> new_focus_control);
  void set_height(int new_height);
  void set_title(const base::string16& new_title);
  void set_width(int new_width);

  void AddFormControl(FormControl* control, ExceptionState* exception_state);

  // ui::SystemMetricsObserver
  void DidChangeIconFont() final;
  void DidChangeSystemColor() final;
  void DidChangeSystemMetrics() final;

  domapi::IntRect bounds_;
  std::vector<FormControl*> controls_;
  gc::Member<FormControl> focus_control_;
  mutable base::ObserverList<FormObserver> observers_;
  base::string16 title_;

  DISALLOW_COPY_AND_ASSIGN(Form);
};

}  // namespace dom

#endif  // EVITA_DOM_FORMS_FORM_H_
