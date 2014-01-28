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
MouseEvent::MouseEvent(EventType event_type, int button_flags, uint32_t flags,
                       const Point& point)
    : Event(event_type),
      client_point_(point),
      button_flags_(button_flags),
      flags_(flags),
      screen_point_(point) {
}

MouseEvent::MouseEvent(EventType event_type, int button_flags, Widget* widget,
                       WPARAM wParam, LPARAM lParam)
    : MouseEvent(event_type, button_flags, GET_KEYSTATE_WPARAM(wParam),
                 MAKEPOINTS(lParam)) {
  WIN32_VERIFY(::MapWindowPoints(widget->AssociatedHwnd(), HWND_DESKTOP,
                                 &screen_point_, 1));
}

MouseEvent::MouseEvent()
    : MouseEvent(EventType::Invalid, 0, 0u, Point()) {
}

MouseEvent::~MouseEvent() {
}

MouseEvent MouseEvent::Create(Widget* widget, uint32_t message, WPARAM wParam,
                              LPARAM lParam) {
  switch (message) {
    // Left button
    case WM_LBUTTONDBLCLK:
        return MouseEvent(EventType::MousePressed, kLeft | 2, widget, wParam,
                          lParam);
    case WM_LBUTTONDOWN:
        return MouseEvent(EventType::MousePressed, kLeft, widget, wParam,
                          lParam);
    case WM_LBUTTONUP:
        return MouseEvent(EventType::MouseReleased, kLeft, widget, wParam,
                          lParam);

    // Middle button
    case WM_MBUTTONDBLCLK:
        return MouseEvent(EventType::MousePressed, kMiddle | 2, widget, wParam,
                          lParam);
    case WM_MBUTTONDOWN:
        return MouseEvent(EventType::MousePressed, kMiddle, widget, wParam,
                          lParam);
    case WM_MBUTTONUP:
        return MouseEvent(EventType::MouseReleased, kMiddle, widget, wParam,
                          lParam);

    // Move
    case WM_MOUSEMOVE:
      return MouseEvent(EventType::MouseMoved, 0, widget, wParam, lParam);

    // Right button
    case WM_RBUTTONDBLCLK:
        return MouseEvent(EventType::MousePressed, kRight | 2, widget, wParam,
                          lParam);
    case WM_RBUTTONDOWN:
        return MouseEvent(EventType::MousePressed, kRight, widget, wParam,
                          lParam);
    case WM_RBUTTONUP:
        return MouseEvent(EventType::MouseReleased, kRight, widget, wParam,
                          lParam);

    // X button
    case WM_XBUTTONDBLCLK:
      if (HIWORD(wParam) == XBUTTON1) {
        return MouseEvent(EventType::MousePressed, kXButton1 | 2, widget,
                          wParam,
                          lParam);
      }
      if (HIWORD(wParam) == XBUTTON2) {
        return MouseEvent(EventType::MousePressed, kXButton2 | 2, widget,
                          wParam, lParam);
      }
      break;
    case WM_XBUTTONDOWN:
      if (HIWORD(wParam) == XBUTTON1) {
        return MouseEvent(EventType::MousePressed, kXButton1, widget,
                          wParam,
                          lParam);
      }
      if (HIWORD(wParam) == XBUTTON2) {
        return MouseEvent(EventType::MousePressed, kXButton2, widget,
                          wParam, lParam);
      }
      break;
    case WM_XBUTTONUP:
      if (HIWORD(wParam) == XBUTTON1) {
        return MouseEvent(EventType::MouseReleased, kXButton1, widget,
                          wParam,
                          lParam);
      }
      if (HIWORD(wParam) == XBUTTON2) {
        return MouseEvent(EventType::MouseReleased, kXButton2, widget,
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
    : MouseEvent(EventType::MouseWheel, 0, GET_KEYSTATE_WPARAM(wParam),
                 MAKEPOINTS(lParam)),
      delta_(GET_WHEEL_DELTA_WPARAM(wParam)) {
  WIN32_VERIFY(::MapWindowPoints(HWND_DESKTOP, widget->AssociatedHwnd(),
                                 &client_point_, 1));
}

MouseWheelEvent::~MouseWheelEvent() {
}

}  // namespace ui
