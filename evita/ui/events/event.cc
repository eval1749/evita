// Copyright (c) 1996-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <iterator>
#include <vector>

#include "evita/ui/events/event.h"

#include "base/logging.h"
#include "common/memory/singleton.h"
#include "common/win/win32_verify.h"
#include "evita/gfx/rect.h"

namespace ui {

namespace {

int current_sequence_number;

// Note: We don't use |IsAltPressed()| in "base/win_util.h", because it links
// netapi32.lib for |NetJoinInformation()| and |NetApiBufferFree()|
bool IsAltPressed() {
  return ::GetKeyState(VK_MENU) < 0;
}

class KeyCodeMapper final : public common::Singleton<KeyCodeMapper> {
  DECLARE_SINGLETON_CLASS(KeyCodeMapper);

 public:
  ~KeyCodeMapper() = default;

  int Map(int virtual_key_code);

 private:
  KeyCodeMapper() : graph_keys_(256) {
    for (auto key_code = 0u; key_code < graph_keys_.size(); ++key_code) {
      auto const char_code = ::MapVirtualKey(key_code, MAPVK_VK_TO_CHAR);
      if (char_code >= 0x20)
        graph_keys_[key_code] = static_cast<int>(char_code);
    }
  }

  std::vector<int> graph_keys_;

  DISALLOW_COPY_AND_ASSIGN(KeyCodeMapper);
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

bool IsNonClientMouseEvent(const base::NativeEvent& native_event) {
  auto const message = native_event.message;
  if (message >= WM_NCMOUSEMOVE && message <= WM_NCMBUTTONDBLCLK)
    return true;
  if (message >= WM_NCXBUTTONDOWN && message <= WM_NCXBUTTONDBLCLK)
    return true;
  if (message == WM_NCMOUSEHOVER && message <= WM_NCMOUSELEAVE)
    return true;
  if (message == WM_NCPOINTERUPDATE && message <= WM_NCPOINTERUP)
    return true;
  return false;
}

}  // namespace

const char* EventTypeName(EventType event_type) {
  static const char* names[] = {
#define V(name) #name,
  FOR_EACH_UI_EVENT_TYPE(V)
#undef V
  };
  auto const it = std::begin(names) + static_cast<size_t>(event_type);
  return it < std::end(names) ? *it : "Unknown";
}

//////////////////////////////////////////////////////////////////////
//
// Event
//
Event::Event(EventType event_type, int flags)
    : default_prevented_(false),
      flags_(flags),
      sequence_number_(++current_sequence_number),
      time_stamp_(base::Time::Now()),
      type_(event_type) {}

Event::Event() : Event(EventType::Invalid, 0) {}

Event::~Event() {}

const char* Event::type_name() const {
  return EventTypeName(type_);
}

void Event::PreventDefault() {
  default_prevented_ = true;
}

//////////////////////////////////////////////////////////////////////
//
// KeyEvent
//
KeyEvent::KeyEvent(EventType event_type, int raw_key_code, bool repeat)
    : Event(event_type, 0), raw_key_code_(raw_key_code), repeat_(repeat) {}

KeyEvent::KeyEvent() : KeyEvent(EventType::Invalid, 0, false) {}

KeyEvent::~KeyEvent() {}

EventType KeyEvent::ConvertToEventType(uint32_t message) {
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

int KeyEvent::ConvertToKeyCode(WPARAM wParam) {
  return KeyCodeMapper::instance()->Map(static_cast<int>(wParam));
}

bool KeyEvent::ConvertToRepeat(LPARAM lParam) {
  return (HIWORD(lParam) & KF_REPEAT) != 0;
}

//////////////////////////////////////////////////////////////////////
//
// MouseEvent
//
MouseEvent::MouseEvent(EventType event_type,
                       MouseButton button,
                       int flags,
                       int click_count,
                       EventTarget* event_target,
                       const gfx::Point& client_point,
                       const gfx::Point& screen_point)
    : Event(event_type, flags),
      button_(button),
      buttons_(ConvertToButtons(flags)),
      click_count_(click_count),
      client_point_(client_point),
      screen_point_(screen_point),
      target_(event_target) {}

MouseEvent::MouseEvent(const base::NativeEvent& native_event,
                       EventTarget* event_target,
                       const gfx::Point& client_point,
                       const gfx::Point& screen_point)
    : MouseEvent(ConvertToEventType(native_event),
                 ConvertToButton(native_event),
                 ConvertToEventFlags(native_event),
                 0,
                 event_target,
                 client_point,
                 screen_point) {}

MouseEvent::MouseEvent()
    : MouseEvent(EventType::Invalid,
                 MouseButton::None,
                 0,
                 0,
                 nullptr,
                 gfx::Point(),
                 gfx::Point()) {}

MouseEvent::~MouseEvent() {}

MouseButton MouseEvent::ConvertToButton(const base::NativeEvent& native_event) {
  switch (native_event.message) {
    // Left button
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
      return MouseButton::Left;

    // Middle button
    case WM_MBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
      return MouseButton::Middle;

    // Move
    case WM_MOUSEMOVE:
      return MouseButton::None;

    // Right button
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
      return MouseButton::Right;

    // X button
    case WM_XBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
      if (HIWORD(native_event.wParam) == XBUTTON1)
        return MouseButton::Other1;
      if (HIWORD(native_event.wParam) == XBUTTON2)
        return MouseButton::Other2;
      break;
  }
  return MouseButton::None;
}

int MouseEvent::ConvertToButtons(int flags) {
  int buttons = 0;
  if (flags & static_cast<int>(EventFlags::LeftButton))
    buttons |= 1 << static_cast<int>(MouseButton::Left);
  if (flags & static_cast<int>(EventFlags::MiddleButton))
    buttons |= 1 << static_cast<int>(MouseButton::Middle);
  if (flags & static_cast<int>(EventFlags::RightButton))
    buttons |= 1 << static_cast<int>(MouseButton::Right);
  if (flags & static_cast<int>(EventFlags::Other1Button))
    buttons |= 1 << static_cast<int>(MouseButton::Other1);
  if (flags & static_cast<int>(EventFlags::Other2Button))
    buttons |= 1 << static_cast<int>(MouseButton::Other2);
  return buttons;
}

int MouseEvent::ConvertToEventFlags(const base::NativeEvent& native_event) {
  if (IsNonClientMouseEvent(native_event))
    return static_cast<int>(EventFlags::NonClient);

  auto flags = 0;
  if (native_event.wParam & MK_CONTROL)
    flags |= static_cast<int>(EventFlags::ControlKey);
  if (native_event.wParam & MK_LBUTTON)
    flags |= static_cast<int>(EventFlags::LeftButton);
  if (native_event.wParam & MK_MBUTTON)
    flags |= static_cast<int>(EventFlags::MiddleButton);
  if (native_event.wParam & MK_RBUTTON)
    flags |= static_cast<int>(EventFlags::RightButton);
  if (native_event.wParam & MK_SHIFT)
    flags |= static_cast<int>(EventFlags::ShiftKey);
  if (native_event.wParam & MK_XBUTTON1)
    flags |= static_cast<int>(EventFlags::Other1Button);
  if (native_event.wParam & MK_XBUTTON1)
    flags |= static_cast<int>(EventFlags::Other2Button);
  if (IsAltPressed())
    flags |= static_cast<int>(EventFlags::AltKey);
  return flags;
}

// Note: Windows sends message in below sequence for double click:
//  1 WM_LBUTTONDOWN
//  2 WM_LBUTTONUP
//  3 WM_LBUTTONDBLCLK
//  4 WM_LBUTTONUP
EventType MouseEvent::ConvertToEventType(
    const base::NativeEvent& native_event) {
  switch (native_event.message) {
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_NCLBUTTONDBLCLK:
    case WM_NCLBUTTONDOWN:
    case WM_NCMBUTTONDBLCLK:
    case WM_NCMBUTTONDOWN:
    case WM_NCRBUTTONDBLCLK:
    case WM_NCRBUTTONDOWN:
    case WM_NCXBUTTONDBLCLK:
    case WM_NCXBUTTONDOWN:
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_XBUTTONDBLCLK:
    case WM_XBUTTONDOWN:
      return EventType::MousePressed;

    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_NCLBUTTONUP:
    case WM_NCMBUTTONUP:
    case WM_NCRBUTTONUP:
    case WM_NCXBUTTONUP:
    case WM_RBUTTONUP:
    case WM_XBUTTONUP:
      return EventType::MouseReleased;

    case WM_MOUSEMOVE:
    case WM_NCMOUSEMOVE:
      return EventType::MouseMoved;
  }
  return EventType::Invalid;
}

//////////////////////////////////////////////////////////////////////
//
// MouseWheelEvent
//
MouseWheelEvent::MouseWheelEvent(EventTarget* event_target,
                                 const gfx::Point& client_point,
                                 const gfx::Point& screen_point,
                                 int flags,
                                 int delta)
    : MouseEvent(EventType::MouseWheel,
                 MouseButton::None,
                 flags,
                 0,
                 event_target,
                 client_point,
                 screen_point),
      delta_(delta) {}

MouseWheelEvent::~MouseWheelEvent() {}

}  // namespace ui

namespace {
const char* MouseButton(const ui::MouseEvent& event) {
  static const char* button_names[] = {
      "Left", "Middle", "Right", "Other1", "Other2",
  };
  if (static_cast<size_t>(event.button()) < arraysize(button_names))
    return button_names[static_cast<int>(event.button())];
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
  return out << ui::EventTypeName(event_type);
}

std::ostream& operator<<(std::ostream& out, const ui::Event& event) {
  return out << event.type_name() << "Event()";
}

std::ostream& operator<<(std::ostream& out, const ui::Event* event) {
  return out << *event;
}

std::ostream& operator<<(std::ostream& out, const ui::KeyEvent& event) {
  return out << event.type_name() << "Event"
             << "(key_code=" << event.key_code()
             << " repeate=" << event.repeat() << ")";
}

std::ostream& operator<<(std::ostream& out, const ui::KeyEvent* event) {
  return out << *event;
}

std::ostream& operator<<(std::ostream& out, const ui::MouseEvent& event) {
  return out << event.type_name() << "Event(" << MouseModifiers(event)
             << MouseButton(event) << " at " << event.location() << ")";
}

std::ostream& operator<<(std::ostream& out, const ui::MouseEvent* event) {
  return out << *event;
}

std::ostream& operator<<(std::ostream& out, const ui::MouseWheelEvent& event) {
  return out << event.type_name() << "Event("
             << " delta=" << event.delta() << ")";
}

std::ostream& operator<<(std::ostream& out, const ui::MouseWheelEvent* event) {
  return out << *event;
}
