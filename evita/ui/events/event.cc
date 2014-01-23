// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/ui/events/event.h"

namespace Command {
uint32_t TranslateKey(uint32_t vkey_code);
}

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// Event
//
Event::Event(EventType event_type) : event_type_(event_type) {
}

Event::Event() : Event(EventType::Invalid) {
}

Event::~Event() {
}

//////////////////////////////////////////////////////////////////////
//
// KeyboardEvent
//
KeyboardEvent::KeyboardEvent(EventType event_type, LPARAM lParam)
    : Event(event_type), raw_key_code_(0),
      repeat_(HIWORD(lParam) & KF_REPEAT) {
}

KeyboardEvent::KeyboardEvent()
    : KeyboardEvent(EventType::Invalid, 0) {
}

KeyboardEvent::~KeyboardEvent() {
}

KeyboardEvent KeyboardEvent::Create(uint32_t message, WPARAM wParam,
                                    LPARAM lParam) {
  if (message == WM_CHAR) {
    auto event = KeyboardEvent(EventType::KeyPressed, lParam);
    event.raw_key_code_ = static_cast<int>(wParam);
    return event.raw_key_code_ < 0x20 ? KeyboardEvent() : event;
  }

  if (message == WM_KEYDOWN) {
    auto event = KeyboardEvent(EventType::KeyPressed, lParam);
    event.raw_key_code_ = static_cast<int>(
        Command::TranslateKey(static_cast<uint32_t>(wParam)));
    return event.raw_key_code_ ? event : KeyboardEvent();
  }

  return KeyboardEvent();
}

}  // namespace ui
