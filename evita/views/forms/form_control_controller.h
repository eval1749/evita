// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_views_forms_form_control_controller_h)
#define INCLUDE_evita_views_forms_form_control_controller_h

#include "evita/ui/controls/control_controller.h"
#include "evita/views/event_source.h"

namespace views {

class FormControlController : public ui::ControlController,
                              protected EventSource {
  public: FormControlController(domapi::EventTargetId event_target_id);
  public: virtual ~FormControlController();

  // ui::ControlController
  public: virtual void DidKillFocus(ui::Control* control,
                                    ui::Widget* focused_widget) override;
  public: virtual void DidRealize(ui::Control* control) override;
  public: virtual void DidSetFocus(ui::Control* control,
                                   ui::Widget* last_focused_widget) override;
  private: virtual void OnKeyPressed(ui::Control* control,
                                     const ui::KeyboardEvent& event) override;
  private: virtual void OnKeyReleased(ui::Control* control,
                                      const ui::KeyboardEvent& event) override;
  private: virtual void OnMouseMoved(ui::Control* control,
                                     const ui::MouseEvent& event) override;
  private: virtual void OnMousePressed(ui::Control* control,
                                       const ui::MouseEvent& event) override;
  private: virtual void OnMouseReleased(ui::Control* control,
                                        const ui::MouseEvent& event) override;
  private: virtual void OnMouseWheel(ui::Control* control,
                                     const ui::MouseWheelEvent& event) override;
  private: virtual void WillDestroyControl(ui::Control* control) override;

  DISALLOW_COPY_AND_ASSIGN(FormControlController);
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_views_forms_form_control_controller_h)
