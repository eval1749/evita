// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_forms_form_control_set_h)
#define INCLUDE_evita_views_forms_form_control_set_h

#include "common/memory/singleton.h"

#include <unordered_map>

#include "evita/dom/public/view_event.h"

namespace ui {
class Control;
class Widget;
}

namespace views {

class FormControlSet : public common::Singleton<FormControlSet> {
  DECLARE_SINGLETON_CLASS(FormControlSet);

  private: std::unordered_map<ui::Control*, domapi::EventTargetId>
      control_map_;
  private: std::unordered_map<domapi::EventTargetId, ui::Control*>
      id_map_;

  private: FormControlSet();
  public: ~FormControlSet();

  // Returns control for |event_target_id|.
  public: ui::Control* MaybeControl(
      domapi::EventTargetId event_target_id) const;

  // Returns event target id for |widget|.
  public: domapi::EventTargetId MaybeControlId(ui::Widget* widget) const;

  public: void Register(ui::Control* control,
                        domapi::EventTargetId target_id);
  public: void Unregister(ui::Control* control);

  DISALLOW_COPY_AND_ASSIGN(FormControlSet);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_views_forms_form_control_set_h)
