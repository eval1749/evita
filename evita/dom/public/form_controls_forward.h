// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_FORM_CONTROLS_FORWARD_H_
#define EVITA_DOM_PUBLIC_FORM_CONTROLS_FORWARD_H_

namespace domapi {

#define FOR_EACH_DOMAPI_FORM_CONTROL(V) \
  V(Button)                             \
  V(Checkbox)                           \
  V(Label)                              \
  V(RadioButton)                        \
  V(TextField)

#define FOR_EACH_DOMAPI_FORM_CONTROL_STATE(V) \
  V(Checked, checked)                         \
  V(Disabled, disabled)                       \
  V(Focused, focused)                         \
  V(Hovered, hovered)                         \
  V(Pressed, pressed)

// Forward declarations
class FormControl;
class FormControlVisitor;
#define V(Name) class Name;
FOR_EACH_DOMAPI_FORM_CONTROL(V)
#undef V

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_FORM_CONTROLS_FORWARD_H_
