// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_WINDOWS_FORM_WINDOW_H_
#define EVITA_DOM_WINDOWS_FORM_WINDOW_H_

#include <utility>

#include "evita/dom/windows/window.h"

#include "evita/dom/forms/form_observer.h"
#include "evita/gc/member.h"

namespace dom {

class Form;
class FormControl;
class FormWindowInit;

namespace bindings {
class FormWindowClass;
}

//////////////////////////////////////////////////////////////////////
//
// FormWindow
//
class FormWindow final : public ginx::Scriptable<FormWindow, Window>,
                         public FormObserver {
  DECLARE_SCRIPTABLE_OBJECT(FormWindow);

 public:
  ~FormWindow() final;

  Form* form() const { return form_.get(); }

 private:
  friend class bindings::FormWindowClass;

  void DidBeginAnimationFrame(const base::TimeTicks& now);
  void RequestAnimationFrame();
  void UpdateHoveredControl(FormControl* control);

  // bindings
  FormWindow(ScriptHost* script_host, Form* form, const FormWindowInit& init);
  FormWindow(ScriptHost* script_host, Form* form);

  // FormObserver
  void DidChangeForm() final;

  // ViewEventTarget
  std::pair<EventTarget*, KeyboardEvent*> TranslateKeyboardEvent(
      const domapi::KeyboardEvent& event) final;

  std::pair<EventTarget*, MouseEvent*> TranslateMouseEvent(
      const domapi::MouseEvent& event) final;

  // Window
  void DidKillFocus() final;
  void DidSetFocus() final;
  void DidShowWindow() final;

  const gc::Member<Form> form_;
  gc::Member<FormControl> hovered_control_;
  bool is_waiting_animation_frame_ = false;

  DISALLOW_COPY_AND_ASSIGN(FormWindow);
};

}  // namespace dom

#endif  // EVITA_DOM_WINDOWS_FORM_WINDOW_H_
