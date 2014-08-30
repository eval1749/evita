// Copyright (c) 2013-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_public_view_event_h)
#define INCLUDE_evita_dom_public_view_event_h

#include "evita/dom/windows/window_id.h"

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
  EventType event_type;
  EventTargetId target_id;
};

struct FocusEvent : Event {
  EventTargetId related_target_id;
};

struct FormEvent : Event {
  int control_id;
  base::string16 data;
};

struct KeyboardEvent : Event {
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

struct WheelEvent : MouseEvent {
  int delta_mode;
  double delta_x;
  double delta_y;
  double delta_z;
};

}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_view_event_h)
