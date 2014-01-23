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
MouseEvent::MouseEvent(EventType event_type, int click_count, uint32_t flags,
                       const Point& point)
    : Event(event_type),
      client_point_(point),
      click_count_(click_count),
      flags_(flags),
      screen_point_(point) {
}

MouseEvent::MouseEvent(EventType event_type, int click_count, Widget* widget,
                       WPARAM wParam, LPARAM lParam)
    : MouseEvent(event_type, click_count, GET_KEYSTATE_WPARAM(wParam),
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
    case WM_LBUTTONDBLCLK:
    case WM_MBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_XBUTTONDBLCLK:
      return MouseEvent(EventType::MousePressed, 2, widget, wParam, lParam);
    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_XBUTTONDOWN:
      return MouseEvent(EventType::MousePressed, 1, widget, wParam, lParam);
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    case WM_XBUTTONUP:
      return MouseEvent(EventType::MouseReleased, 1, widget, wParam, lParam);
    case WM_MOUSEMOVE:
      return MouseEvent(EventType::MouseMove, 0, widget, wParam, lParam);
    case WM_MOUSEWHEEL:
      return MouseWheelEvent(widget, wParam, lParam);
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
