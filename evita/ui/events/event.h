// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_ui_events_event_h)
#define INCLUDE_evita_ui_events_event_h

#include <stdint.h>

#include "base/event_types.h"
#include "base/time/time.h"
#include "common/castable.h"
#include "common/win/rect.h"

namespace ui {

using common::win::Point;
using common::win::Rect;
class Widget;

enum class EventFlags {
  None = 0,
  LeftButton = 1 << 1,
  MiddleButton = 1 << 2,
  RightButton = 1 << 3,
  ControlKey = 1 << 4,
  ShiftKey = 1 << 5,
  AltKey = 1 << 6,
  NonClient = 1 << 7,
  Other1Button = 1 << 8,
  Other2Button = 1 << 9,
};

enum class EventType {
  Invalid,
  KeyPressed,
  KeyReleased,
  // Note: We don't have MouseEntered event, because Windows doesn't have it.
  MouseExited,
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
  Pause = VK_PAUSE | 0x100,
  Tab = VK_TAB | 0x100,
};

enum class Modifier {
  None = 0,
  Control = 0x200,
  Shift = 0x400,
  Alt = 0x800,
};

enum class MouseButton {
  None = 0,
  Left = 0,
  Middle = 1,
  Right = 2,
  Other1 = 3,
  Other2 = 4,
};

//////////////////////////////////////////////////////////////////////
//
// Event
//
class Event : public common::Castable {
  DECLARE_CASTABLE_CLASS(Event, Castable);

  private: bool default_prevented_;
  private: EventType event_type_;
  private: int flags_;
  private: base::Time time_stamp_;

  protected: Event(EventType event_type, int flags);
  protected: Event();
  public: virtual ~Event();

  public: bool default_prevented() const { return default_prevented_; }
  public: EventType event_type() const { return event_type_; }
  public: int flags() const { return flags_; }
  public: base::Time time_stamp() const { return time_stamp_; }

  public: void PreventDefault();
};

//////////////////////////////////////////////////////////////////////
//
// KeyboardEvent
//
class KeyboardEvent : public Event {
  DECLARE_CASTABLE_CLASS(KeyboardEvent, Event);

  private: int raw_key_code_;
  private: bool repeat_;

  public: KeyboardEvent(EventType type, int key_code, bool repeat);
  public: KeyboardEvent();
  public: virtual ~KeyboardEvent();

  public: const bool alt_key() const {
    return raw_key_code_ & static_cast<int>(Modifier::Alt);
  }
  public: const bool control_key() const {
    return raw_key_code_ & static_cast<int>(Modifier::Control);
  }
  public: int key_code() const { return raw_key_code_ & 0x1FF; }
  public: int raw_key_code() const { return raw_key_code_; }
  public: bool repeat() const { return repeat_; }

  public: const bool shift_key() const {
    return raw_key_code_ & static_cast<int>(Modifier::Shift);
  }

  public: static EventType ConvertToEventType(uint32_t message);
  public: static int ConvertToKeyCode(WPARAM wParam);
  public: static bool ConvertToRepeat(LPARAM lParam);
};

//////////////////////////////////////////////////////////////////////
//
// MouseEvent
//
class MouseEvent : public Event {
  DECLARE_CASTABLE_CLASS(MouseEvent, Event);

  friend class MouseWheelEvent;

  private: MouseButton button_;
  private: int buttons_;
  private: int click_count_;
  private: Point client_point_;
  private: Point screen_point_;
  private: Widget* target_;

  public: MouseEvent(EventType type, MouseButton button, int flags,
                     int click_count, Widget* widget,
                     const Point& client_point, const Point& screen_point);
  public: MouseEvent(const base::NativeEvent& native_event, Widget* widget,
                     const Point& client_point, const Point& screen_point);
  public: MouseEvent();
  public: virtual ~MouseEvent();

  public: bool alt_key() const {
    return flags() & static_cast<int>(EventFlags::AltKey);
  }
  public: MouseButton button() const { return button_; }
  public: int buttons() const { return buttons_; }
  public: int click_count() const { return click_count_; }
  public: bool control_key() const {
    return flags() & static_cast<int>(EventFlags::ControlKey);
  }
  public: bool is_left_button() const { return button_ == MouseButton::Left; }
  public: bool is_middle_button() const {
    return button_ == MouseButton::Middle;
  }
  public: bool is_non_client() const {
    return flags() & static_cast<int>(EventFlags::NonClient);
  }
  public: bool is_right_button() const { return button_ == MouseButton::Right; }
  public: bool is_other1_button() const {
    return button_ == MouseButton::Other1;
  }
  public: bool is_other2_button() const {
    return button_ == MouseButton::Other2;
  }
  public: Point location() const { return client_point_; }
  public: Point screen_location() const { return screen_point_; }
  public: bool shift_key() const {
    return flags() & static_cast<int>(EventFlags::ShiftKey);
  }
  public: Widget* target() const { return target_; }

  public: static MouseButton ConvertToButton(
      const base::NativeEvent& native_event);
  public: static int ConvertToButtons(int flags);
  public: static EventType ConvertToEventType(
      const base::NativeEvent& native_event);
  public: static int ConvertToEventFlags(
      const base::NativeEvent& native_event);
};

//////////////////////////////////////////////////////////////////////
//
// MouseWheelEvent
//
class MouseWheelEvent : public MouseEvent {
  DECLARE_CASTABLE_CLASS(MouseEvent, MouseEvent);

  friend class MouseEvent;

  private: int delta_;

  public: MouseWheelEvent(Widget* widget, const Point& client_point,
                          const Point& screen_point, int flags,
                          int delta);
  public: virtual ~MouseWheelEvent();

  public: int delta() const { return delta_; }
};

}  // namespace ui

#include <ostream>

std::ostream& operator<<(std::ostream& out, ui::EventType event_type);
std::ostream& operator<<(std::ostream& out, const ui::Event& event);
std::ostream& operator<<(std::ostream& out, const ui::Event* event);
std::ostream& operator<<(std::ostream& out, const ui::KeyboardEvent& event);
std::ostream& operator<<(std::ostream& out, const ui::KeyboardEvent* event);
std::ostream& operator<<(std::ostream& out, const ui::MouseEvent& event);
std::ostream& operator<<(std::ostream& out, const ui::MouseEvent* event);
std::ostream& operator<<(std::ostream& out, const ui::MouseWheelEvent& event);
std::ostream& operator<<(std::ostream& out, const ui::MouseWheelEvent* event);

#endif // !defined(INCLUDE_evita_ui_events_event_h)
