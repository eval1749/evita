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

struct MouseEvent {
  EventType type;
  dom::WindowId target_id;
  int client_x;
  int client_y;
  MouseButton button;
  bool alt_key;
  bool control_key;
  bool shift_key;
};

}  // namespace domapi

#endif //!defined(INCLUDE_evita_dom_public_api_event_h)
