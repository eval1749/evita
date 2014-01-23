// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "base/basictypes.h"
#include "evita/ui/events/event_logging.h"

namespace {
const char* event_names[] = {
  "Invalid",
  "KeyDown",
  "KeyUp",
  "MouseDown",
  "MouseMove",
  "MouseUp",
  "MouseWheel",
};
}

std::ostream& operator<<(std::ostream& out, ui::EventType event_type) {
  auto index = static_cast<size_t>(event_type);
  if (index < arraysize(event_names)) {
    return out << event_names[index];
  }
  return out << index;
}

std::ostream& operator<<(std::ostream& out, const ui::Event& event) {
  return out << event.event_type() << "Event()";
}

std::ostream& operator<<(std::ostream& out, const ui::Event* event) {
  return out << *event;
}


std::ostream& operator<<(std::ostream& out, const ui::KeyboardEvent& event) {
  return out << event.event_type() << "Event(" <<
      " key_code=" << event.key_code() <<
      " repeate=" << event.repeat() << ")";
}

std::ostream& operator<<(std::ostream& out, const ui::KeyboardEvent* event) {
  return out << *event;
}
