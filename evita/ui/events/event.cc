// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/ui/events/event.h"

#include <algorithm>
#include <vector>

#include "base/logging.h"
#include "common/memory/singleton.h"
#include "common/win/win32_verify.h"
#include "evita/ui/widget.h"

namespace ui {

namespace {

class KeyCodeMapper : public common::Singleton<KeyCodeMapper> {
  DECLARE_SINGLETON_CLASS(KeyCodeMapper);

  private: std::vector<int> graph_keys_;

  private: KeyCodeMapper() : graph_keys_(256) {
    for (auto key_code = 0u; key_code < graph_keys_.size(); ++key_code) {
      auto const char_code = ::MapVirtualKey(key_code, MAPVK_VK_TO_CHAR);
      if (char_code >= 0x20)
        graph_keys_[key_code] = static_cast<int>(char_code);
    }
  }

  public: ~KeyCodeMapper() = default;

  public: int Map(int virtual_key_code);
};

int KeyCodeMapper::Map(int virtual_key_code) {
  if (VK_CONTROL == virtual_key_code)
    return 0;
  if (VK_MENU == virtual_key_code)
    return 0;
  if (VK_SHIFT == virtual_key_code)
    return 0;

  auto key_code = graph_keys_[static_cast<size_t>(virtual_key_code)];

  if (!key_code) {
   if (virtual_key_code == VK_CANCEL)
     key_code = VK_PAUSE | 0x100;
   else
     key_code = virtual_key_code | 0x100;
  }

  if (::GetKeyState(VK_MENU) < 0)
    key_code |= static_cast<int>(Modifier::Alt);
  if (::GetKeyState(VK_CONTROL) < 0)
    key_code |= static_cast<int>(Modifier::Control);
  if (::GetKeyState(VK_SHIFT) < 0)
    key_code |= static_cast<int>(Modifier::Shift);

  return key_code;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Event
//
Event::Event(EventType event_type)
    : event_type_(event_type), time_stamp_(base::Time::Now()) {
}

Event::Event() : Event(EventType::Invalid) {
}

Event::~Event() {
}

//////////////////////////////////////////////////////////////////////
//
// KeyboardEvent
//
KeyboardEvent::KeyboardEvent(EventType event_type, int raw_key_code,
                             bool repeat)
    : Event(event_type), raw_key_code_(raw_key_code), repeat_(repeat) {
}

KeyboardEvent::KeyboardEvent()
    : KeyboardEvent(EventType::Invalid, 0, false) {
}

KeyboardEvent::~KeyboardEvent() {
}

EventType KeyboardEvent::ConvertToEventType(uint32_t message) {
  switch (message) {
    case WM_CHAR:
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
      return EventType::KeyPressed;
    case WM_KEYUP:
    case WM_SYSKEYUP:
      return EventType::KeyReleased;
  }
  return EventType::Invalid;
}

int KeyboardEvent::ConvertToKeyCode(WPARAM wParam) {
  return KeyCodeMapper::instance()->Map(static_cast<int>(wParam));
}

bool KeyboardEvent::ConvertToRepeat(LPARAM lParam) {
  return HIWORD(lParam) & KF_REPEAT;
}

//////////////////////////////////////////////////////////////////////
//
// MouseEvent
//
MouseEvent::MouseEvent(EventType event_type, Button button, uint32_t flags,
                       int click_count, Widget* widget,
                       const Point& client_point, const Point& screen_point)
    : Event(event_type),
      alt_key_(false),
      button_(button),
      buttons_(ConvertToButtons(flags)),
      click_count_(click_count),
      client_point_(client_point),
      control_key_(flags & MK_CONTROL),
      screen_point_(screen_point),
      shift_key_(flags & MK_SHIFT),
      target_(widget) {
}

MouseEvent::MouseEvent()
    : MouseEvent(EventType::Invalid, kNone, 0u, 0, nullptr, Point(), Point()) {
}

MouseEvent::~MouseEvent() {
}

int MouseEvent::ConvertToButtons(uint32_t flags) {
  int buttons = 0;
  if (flags & MK_LBUTTON)
    buttons |= 1 << MouseEvent::kLeft;
  if (flags & MK_MBUTTON)
    buttons |= 1 << MouseEvent::kMiddle;
  if (flags & MK_RBUTTON)
    buttons |= 1 << MouseEvent::kRight;
  if (flags & MK_XBUTTON1)
    buttons |= 1 << MouseEvent::kOther1;
  if (flags & MK_XBUTTON2)
    buttons |= 1 << MouseEvent::kOther2;
  return buttons;
}

MouseEvent::Button MouseEvent::ConvertToButton(uint32_t message,
                                               WPARAM wParam) {
  switch (message) {
    // Left button
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        return kLeft;

    // Middle button
    case WM_MBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        return kMiddle;

    // Move
    case WM_MOUSEMOVE:
      return kNone;

    // Right button
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
      return kRight;

    // X button
    case WM_XBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
      if (HIWORD(wParam) == XBUTTON1)
        return kOther1;
      if (HIWORD(wParam) == XBUTTON2)
        return kOther2;
      break;
  }
  return kNone;
}

// Note: Windows sends message in below sequence for double click:
//  1 WM_LBUTTONDOWN
//  2 WM_LBUTTONUP
//  3 WM_LBUTTONDBLCLK
//  4 WM_LBUTTONUP
EventType MouseEvent::ConvertToEventType(uint32_t message) {
  switch (message) {
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_XBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
        return EventType::MousePressed;

    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_XBUTTONUP:
      return EventType::MouseReleased;

    case WM_MOUSEMOVE:
      return EventType::MouseMoved;
  }
  return EventType::Invalid;
}

//////////////////////////////////////////////////////////////////////
//
// MouseWheelEvent
//
MouseWheelEvent::MouseWheelEvent(Widget* widget, const Point& client_point,
                                 const Point& screen_point, uint32_t flags,
                                 int delta)
    : MouseEvent(EventType::MouseWheel, kNone, flags, 0, widget, client_point,
                 screen_point), delta_(delta) {
}

MouseWheelEvent::~MouseWheelEvent() {
}

}  // namespace ui

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
