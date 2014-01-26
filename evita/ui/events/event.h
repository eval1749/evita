// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_ui_events_event_h)
#define INCLUDE_evita_ui_events_event_h

#include <stdint.h>

#include "common/win/rect.h"

namespace ui {

using common::win::Point;
using common::win::Rect;
class Widget;

enum class EventType {
  Invalid,
  KeyPressed,
  KeyReleased,
  MouseMoved,
  MousePressed,
  MouseReleased,
  MouseWheel,
};

enum class KeyCode {
  ArrowDown = VK_DOWN | 0x100,
  ArrowLeft = VK_LEFT | 0x100,
  ArrowRight = VK_LEFT | 0x100,
  ArrowUp = VK_UP | 0x100,
  Backspace = VK_BACK | 0x100,
  Delete = VK_DELETE | 0x100,
  End = VK_END | 0x100,
  Enter = VK_RETURN | 0x100,
  Home = VK_HOME | 0x100,
  Insert = VK_INSERT | 0x100,
  PageDown  = VK_NEXT | 0x100,
  PageUp = VK_PRIOR | 0x100,
  Tab = VK_TAB | 0x100,
};

enum class Modifier {
  None = 0,
  Control = 0x200,
  Shift = 0x400,
};

//////////////////////////////////////////////////////////////////////
//
// Event
//
class Event {
  private: EventType event_type_;

  protected: explicit Event(EventType event_type);
  protected: Event();
  public: ~Event();

  public: EventType event_type() const { return event_type_; }
};

//////////////////////////////////////////////////////////////////////
//
// KeyboardEvent
//
class KeyboardEvent : public Event {
  private: int raw_key_code_;
  private: bool repeat_;

  private: KeyboardEvent(EventType type, LPARAM lParam);
  private: KeyboardEvent();
  public: ~KeyboardEvent();

  public: const bool control_key() const {
    return raw_key_code_ & static_cast<int>(Modifier::Control);
  }
  public: int key_code() const { return raw_key_code_ & 0x1FF; }
  public: int raw_key_code() const { return raw_key_code_; }
  public: bool repeat() const { return repeat_; }

  public: const bool shift_key() const {
    return raw_key_code_ & static_cast<int>(Modifier::Shift);
  }

  public: static KeyboardEvent Create(uint32_t message, WPARAM wParam,
                                      LPARAM lParam);
};

//////////////////////////////////////////////////////////////////////
//
// MouseEvent
//
class MouseEvent : public Event {
  friend class MouseWheelEvent;

  private: enum ButtonFlags {
    kClickMask = 3,
    kLeft = 1 << 2,
    kMiddle = 1 << 3,
    kRight = 1 << 4,
    kXButton1 = 1 << 5,
    kXButton2 = 1 << 6,
  };

  private: int button_flags_;
  private: uint32_t flags_;
  private: Point client_point_;
  private: Point screen_point_;
  protected: MouseEvent(EventType type, int button_flags, uint32_t flags,
                        const Point& point);
  private: MouseEvent(EventType type, int button_flags, Widget* widget,
                      WPARAM wParam, LPARAM lParam);
  private: MouseEvent();
  public: ~MouseEvent();

  public: int click_count() const { return button_flags_ & kClickMask; }
  public: bool control_key() const { return flags_ & MK_CONTROL; }
  public: uint32_t flags() const { return flags_; }
  public: bool is_left_button() const { return button_flags_ & kLeft; }
  public: bool is_middle_button() const { return button_flags_ & kMiddle; }
  public: bool is_right_button() const { return button_flags_ & kRight; }
  public: bool is_x_button1() const { return button_flags_ & kXButton1; }
  public: bool is_x_button2() const { return button_flags_ & kXButton2; }
  public: Point location() const { return client_point_; }
  public: bool shift_key() const { return flags_ & MK_SHIFT; }

  public: static MouseEvent Create(Widget* widget, uint32_t message,
                                   WPARAM wParam, LPARAM lParam);
};

//////////////////////////////////////////////////////////////////////
//
// MouseWheelEvent
//
class MouseWheelEvent : public MouseEvent {
  friend class MouseEvent;

  private: int delta_;

  private: MouseWheelEvent(Widget* widget, WPARAM wParam, LPARAM lParam);
  public: ~MouseWheelEvent();

  public: int delta() const { return delta_; }
};

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_events_event_h)
