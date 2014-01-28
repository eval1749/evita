// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/ui/events/event.h"

#include "common/win/win32_verify.h"
#include "evita/ui/widget.h"

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

//////////////////////////////////////////////////////////////////////
//
// MouseEvent
//
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
