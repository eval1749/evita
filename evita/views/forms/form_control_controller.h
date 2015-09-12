// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_VIEWS_FORMS_FORM_CONTROL_CONTROLLER_H_
#define EVITA_VIEWS_FORMS_FORM_CONTROL_CONTROLLER_H_

#include "evita/ui/base/ime/text_input_delegate.h"
#include "evita/ui/controls/control_controller.h"
#include "evita/views/event_source.h"

namespace views {

class FormControlController final : public ui::ControlController,
                                    public ui::TextInputDelegate,
                                    protected EventSource {
 public:
  explicit FormControlController(domapi::EventTargetId event_target_id);
  ~FormControlController();

  // ui::ControlController
  void DidKillFocus(ui::Control* control, ui::Widget* focused_widget) final;
  void DidRealize(ui::Control* control) final;
  void DidSetFocus(ui::Control* control, ui::Widget* last_focused_widget) final;

 private:
  void OnKeyEvent(ui::Control* control, const ui::KeyEvent& event) final;
  void OnMouseEvent(ui::Control* control, const ui::MouseEvent& event) final;
  void WillDestroyControl(ui::Control* control) final;

  // ui::TextInputDelegate
  void DidCommitComposition(const ui::TextComposition& composition) final;
  void DidFinishComposition() final;
  void DidStartComposition() final;
  void DidUpdateComposition(const ui::TextComposition& composition) final;
  ui::Widget* GetClientWindow() final;

  DISALLOW_COPY_AND_ASSIGN(FormControlController);
};

}  // namespace views

#endif  // EVITA_VIEWS_FORMS_FORM_CONTROL_CONTROLLER_H_
