// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_ui_events_event_ostream_h)
#define INCLUDE_evita_ui_events_event_ostream_h

#include <ostream>

#include "evita/ui/events/event.h"

std::ostream& operator<<(std::ostream& out, ui::EventType event_type);
std::ostream& operator<<(std::ostream& out, const ui::Event& event);
std::ostream& operator<<(std::ostream& out, const ui::Event* event);
std::ostream& operator<<(std::ostream& out, const ui::KeyboardEvent& event);
std::ostream& operator<<(std::ostream& out, const ui::KeyboardEvent* event);
std::ostream& operator<<(std::ostream& out, const ui::MouseEvent& event);
std::ostream& operator<<(std::ostream& out, const ui::MouseEvent* event);
std::ostream& operator<<(std::ostream& out, const ui::MouseWheelEvent& event);
std::ostream& operator<<(std::ostream& out, const ui::MouseWheelEvent* event);

#endif //!defined(INCLUDE_evita_ui_events_event_ostream_h)
