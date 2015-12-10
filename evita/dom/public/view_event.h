// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PUBLIC_VIEW_EVENT_H_
#define EVITA_DOM_PUBLIC_VIEW_EVENT_H_

#include "base/strings/string16.h"
#include "evita/dom/public/window_id.h"

namespace domapi {

enum class EventType {
  Invalid,
  Blur,
  Click,
  DblClick,
  Focus,
  FormChange,
  FormClick,
  KeyDown,
  KeyUp,
  Load,
  MouseDown,
  MouseEnter,
  MouseLeave,
  MouseMove,
  MouseUp,
  TextCompositionCancel,
  TextCompositionCommit,
  TextCompositionEnd,
  TextCompositionStart,
  TextCompositionUpdate,
  Wheel,
};

enum class Modifier {
  Control = 1,
  Shift = 2,
  Alt = 4,
};

enum class MouseButton {
  Left,
  Middle,
  Right,
  Other1,
  Other2,
};

enum class Visibility {
  Hidden,
  Visible,
};

struct Event {
  int event_id;
  EventType event_type;
  EventTargetId target_id;
};

struct FocusEvent final : Event {
  EventTargetId related_target_id;
};

struct FormEvent final : Event {
  int control_id;
  base::string16 data;
};

struct KeyboardEvent final : Event {
  bool alt_key;
  bool control_key;
  int key_code;
  int location;
  int repeat;
  bool meta_key;
  bool shift_key;
};

struct MouseEvent : Event {
  bool alt_key;
  MouseButton button;
  int buttons;
  int client_x;
  int client_y;
  bool control_key;
  bool shift_key;
};

struct WheelEvent final : MouseEvent {
  int delta_mode;
  double delta_x;
  double delta_y;
  double delta_z;
};

}  // namespace domapi

#endif  // EVITA_DOM_PUBLIC_VIEW_EVENT_H_
