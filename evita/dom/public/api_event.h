// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_public_api_event_h)
#define INCLUDE_evita_dom_public_api_event_h

#include "evita/dom/window_id.h"

namespace domapi {

enum class EventType {
  Invalid,
  Blur,
  Click,
  DblClick,
  Focus,
  KeyDown,
  KeyUp,
  Load,
  MouseDown,
  MouseMove,
  MouseUp,
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

struct FormEvent {
  dom::EventTargetId target_id;
  int control_id;
  base::string16 type;
  base::string16 data;
};

struct KeyboardEvent {
  bool alt_key;
  bool control_key;
  EventType event_type;
  int key_code;
  int location;
  int repeat;
  bool meta_key;
  bool shift_key;
  dom::WindowId target_id;
};

struct MouseEvent {
  bool alt_key;
  MouseButton button;
  int buttons;
  int client_x;
  int client_y;
  bool control_key;
  EventType event_type;
  bool shift_key;
  dom::WindowId target_id;
};

struct WheelEvent : MouseEvent {
  int delta_mode;
  double delta_x;
  double delta_y;
  double delta_z;
};

}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_api_event_h)
