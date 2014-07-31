// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "base/basictypes.h"
#include "evita/ui/events/event_ostream.h"

namespace {
const char* event_names[] = {
  "Invalid",
  "KeyPressed",
  "KeyReleased",
  "MouseMoved",
  "MousePressed",
  "MouseReleased",
  "MouseWheel",
};

const char* MouseButton(const ui::MouseEvent& event) {
  static const char* button_names[] = {
    "Left", "Middle", "Right", "Other1", "Other2",
  };
  if (static_cast<size_t>(event.button()) < arraysize(button_names))
    return button_names[event.button()];
  return "?";
}

const char* MouseModifiers(const ui::MouseEvent& event) {
  static const char* modifier_names[] = {
    "", "Ctrl+", "Shift+", "Ctrl+Shift+",
  };
  static_assert(sizeof(modifier_names) == sizeof(const char*) * 4,
                "arraysize(modifier_names[]) must be 4.");
  auto index = 0;
  if (event.control_key())
    index |= 1;
  if (event.shift_key())
    index |= 2;
  return modifier_names[index];
}
}  // namespace

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
  return out << event.event_type() << "Event" <<
      "(key_code=" << event.key_code() <<
      " repeate=" << event.repeat() << ")";
}

std::ostream& operator<<(std::ostream& out, const ui::KeyboardEvent* event) {
  return out << *event;
}

std::ostream& operator<<(std::ostream& out, const ui::MouseEvent& event) {
  return out << event.event_type() << "Event(" <<
      MouseModifiers(event) << MouseButton(event) <<
      " at " << event.location() << ")";
}

std::ostream& operator<<(std::ostream& out, const ui::MouseEvent* event) {
  return out << *event;
}

std::ostream& operator<<(std::ostream& out, const ui::MouseWheelEvent& event) {
  return out << event.event_type() << "Event(" <<
      " delta=" << event.delta() << ")";
}

std::ostream& operator<<(std::ostream& out, const ui::MouseWheelEvent* event) {
  return out << *event;
}
