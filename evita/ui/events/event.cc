// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/ui/events/event.h"

#include <vector>

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
  if (VK_SHIFT == virtual_key_code)
    return 0;

  auto key_code = graph_keys_[static_cast<size_t>(virtual_key_code)];

  if (!key_code) {
   if (virtual_key_code == VK_CANCEL)
     key_code = VK_PAUSE | 0x100;
   else
     key_code = virtual_key_code | 0x100;
  }

  if (::GetKeyState(VK_CONTROL) < 0) {
    key_code |= static_cast<int>(Modifier::Control);
  } else if (key_code <= 0xFF) {
    // For graphics key, we use WM_CHAR.
    return 0;
  }

  if (::GetKeyState(VK_SHIFT) < 0)
    key_code |= static_cast<int>(Modifier::Shift);
  return key_code;
}

}  // namespace

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
    event.raw_key_code_ = KeyCodeMapper::instance()->
        Map(static_cast<int>(wParam));
    return event.raw_key_code_ ? event : KeyboardEvent();
  }

  return KeyboardEvent();
}

//////////////////////////////////////////////////////////////////////
//
// MouseEvent
//
MouseEvent::MouseEvent(EventType event_type, const Point& screen_point,
                       const Point& client_point)
    : Event(event_type),
      alt_key_(false),
      button_(0),
      buttons_(0),
      client_point_(client_point),
      control_key_(false),
      screen_point_(screen_point),
      shift_key_(false) {
}

MouseEvent::MouseEvent(EventType event_type, Button button, int click_count,
                       uint32_t flags, const Point& point)
    : Event(event_type),
      alt_key_(false),
      button_(button),
      buttons_(ConvertToButtons(flags)),
      click_count_(click_count),
      client_point_(point),
      control_key_(flags & MK_CONTROL),
      screen_point_(point),
      shift_key_(flags & MK_SHIFT) {
}

MouseEvent::MouseEvent(EventType event_type, Button button, int click_count,
                       Widget* widget, WPARAM wParam, LPARAM lParam)
    : MouseEvent(event_type, button, click_count, GET_KEYSTATE_WPARAM(wParam),
                 MAKEPOINTS(lParam)) {
  WIN32_VERIFY(::MapWindowPoints(widget->AssociatedHwnd(), HWND_DESKTOP,
                                 &screen_point_, 1));
}

MouseEvent::MouseEvent()
    : MouseEvent(EventType::Invalid, kNone, 0, 0u, Point()) {
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


MouseEvent MouseEvent::Create(Widget* widget, uint32_t message, WPARAM wParam,
                              LPARAM lParam) {
  switch (message) {
    // Left button
    case WM_LBUTTONDBLCLK:
        return MouseEvent(EventType::MousePressed, kLeft, 2, widget, wParam,
                          lParam);
    case WM_LBUTTONDOWN:
        return MouseEvent(EventType::MousePressed, kLeft, 0, widget, wParam,
                          lParam);
    case WM_LBUTTONUP:
        return MouseEvent(EventType::MouseReleased, kLeft, 0, widget, wParam,
                          lParam);

    // Middle button
    case WM_MBUTTONDBLCLK:
        return MouseEvent(EventType::MousePressed, kMiddle, 2, widget, wParam,
                          lParam);
    case WM_MBUTTONDOWN:
        return MouseEvent(EventType::MousePressed, kMiddle, 0, widget, wParam,
                          lParam);
    case WM_MBUTTONUP:
        return MouseEvent(EventType::MouseReleased, kMiddle, 0, widget, wParam,
                          lParam);

    // Move
    case WM_MOUSEMOVE:
      return MouseEvent(EventType::MouseMoved, kNone, 0, widget, wParam,
                        lParam);

    // Right button
    case WM_RBUTTONDBLCLK:
        return MouseEvent(EventType::MousePressed, kRight, 2, widget, wParam,
                          lParam);
    case WM_RBUTTONDOWN:
        return MouseEvent(EventType::MousePressed, kRight, 0, widget, wParam,
                          lParam);
    case WM_RBUTTONUP:
        return MouseEvent(EventType::MouseReleased, kRight, 0, widget, wParam,
                          lParam);

    // X button
    case WM_XBUTTONDBLCLK:
      if (HIWORD(wParam) == XBUTTON1) {
        return MouseEvent(EventType::MousePressed, kOther1, 2, widget,
                          wParam,
                          lParam);
      }
      if (HIWORD(wParam) == XBUTTON2) {
        return MouseEvent(EventType::MousePressed, kOther2, 2, widget,
                          wParam, lParam);
      }
      break;
    case WM_XBUTTONDOWN:
      if (HIWORD(wParam) == XBUTTON1) {
        return MouseEvent(EventType::MousePressed, kOther1, 0, widget,
                          wParam, lParam);
      }
      if (HIWORD(wParam) == XBUTTON2) {
        return MouseEvent(EventType::MousePressed, kOther2, 0, widget,
                          wParam, lParam);
      }
      break;
    case WM_XBUTTONUP:
      if (HIWORD(wParam) == XBUTTON1) {
        return MouseEvent(EventType::MouseReleased, kOther1, 0, widget,
                          wParam, lParam);
      }
      if (HIWORD(wParam) == XBUTTON2) {
        return MouseEvent(EventType::MouseReleased, kOther2, 0, widget,
                          wParam, lParam);
      }
      break;
  }
  return MouseEvent();
}

//////////////////////////////////////////////////////////////////////
//
// MouseWheelEvent
//
MouseWheelEvent::MouseWheelEvent(Widget* widget, WPARAM wParam, LPARAM lParam)
    : MouseEvent(EventType::MouseWheel, kNone, 0, GET_KEYSTATE_WPARAM(wParam),
                 MAKEPOINTS(lParam)),
      delta_(GET_WHEEL_DELTA_WPARAM(wParam)) {
  WIN32_VERIFY(::MapWindowPoints(HWND_DESKTOP, widget->AssociatedHwnd(),
                                 &client_point_, 1));
}

MouseWheelEvent::~MouseWheelEvent() {
}

}  // namespace ui
