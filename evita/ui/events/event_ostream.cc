// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "base/basictypes.h"
#include "common/win/point_ostream.h"
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

const char* MouseButtons(uint32_t flags) {
  static const char* button_names[] = {
    "",  "L", "M", "LM", "R", "LR", "MR", "LMR",
  };
  static_assert(sizeof(button_names) == sizeof(const char*) * 8,
                "arraysize(button_names[]) must be 8.");
  auto index = 0;
  if (flags & MK_LBUTTON)
    index |= 1;
  if (flags & MK_MBUTTON)
    index |= 2;
  if (flags & MK_RBUTTON)
    index |= 4;
  return button_names[index];
}

const char* MouseModifiers(uint32_t flags) {
  static const char* modifier_names[] = {
    "", "Ctrl", "Shift", "Ctrl+Shift",
  };
  static_assert(sizeof(modifier_names) == sizeof(const char*) * 4,
                "arraysize(modifier_names[]) must be 4.");
  auto index = 0;
  if (flags & MK_CONTROL)
    index |= 1;
  if (flags & MK_SHIFT)
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
  return out << event.event_type() << "Event" <<
      "(button=" << MouseButtons(event.flags()) <<
      " modifiers=" << MouseModifiers(event.flags()) <<
      " count=" << event.click_count() <<
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
