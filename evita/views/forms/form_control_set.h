// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_FORMS_FORM_CONTROL_SET_H_
#define EVITA_VIEWS_FORMS_FORM_CONTROL_SET_H_

#include <unordered_map>

#include "base/macros.h"
#include "common/memory/singleton.h"
#include "evita/dom/public/view_event.h"

namespace ui {
class Control;
class Widget;
}

namespace views {

class FormControlSet : public common::Singleton<FormControlSet> {
  DECLARE_SINGLETON_CLASS(FormControlSet);

 public:
  ~FormControlSet();

  // Returns control for |event_target_id|.
  ui::Control* MaybeControl(domapi::EventTargetId event_target_id) const;

  // Returns event target id for |widget|.
  domapi::EventTargetId MaybeControlId(ui::Widget* widget) const;

  void Register(ui::Control* control, domapi::EventTargetId target_id);
  void Unregister(ui::Control* control);

 private:
  FormControlSet();

  std::unordered_map<ui::Control*, domapi::EventTargetId> control_map_;
  std::unordered_map<domapi::EventTargetId, ui::Control*> id_map_;

  DISALLOW_COPY_AND_ASSIGN(FormControlSet);
};

}  // namespace views

#endif  // EVITA_VIEWS_FORMS_FORM_CONTROL_SET_H_
