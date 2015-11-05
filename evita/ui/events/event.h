// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_UI_EVENTS_EVENT_H_
#define EVITA_UI_EVENTS_EVENT_H_

#include <stdint.h>
#include <ostream>

#include "base/event_types.h"
#include "base/time/time.h"
#include "common/castable.h"
#include "common/win/rect.h"

namespace ui {

using common::win::Point;
using common::win::Rect;
class EventTarget;

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
  MouseEntered,
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
  PageDown = VK_NEXT | 0x100,
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

 public:
  ~Event() override;

  bool default_prevented() const { return default_prevented_; }
  EventType event_type() const { return event_type_; }
  int id() const { return sequence_number_; }
  int flags() const { return flags_; }
  base::Time time_stamp() const { return time_stamp_; }

  void PreventDefault();

 protected:
  Event(EventType event_type, int flags);
  Event();

 private:
  bool default_prevented_;
  EventType event_type_;
  int flags_;
  int sequence_number_;
  base::Time time_stamp_;
};

//////////////////////////////////////////////////////////////////////
//
// KeyEvent
//
class KeyEvent final : public Event {
  DECLARE_CASTABLE_CLASS(KeyEvent, Event);

 public:
  KeyEvent(EventType type, int key_code, bool repeat);
  KeyEvent();
  ~KeyEvent() final;

  const bool alt_key() const {
    return (raw_key_code_ & static_cast<int>(Modifier::Alt)) != 0;
  }
  const bool control_key() const {
    return (raw_key_code_ & static_cast<int>(Modifier::Control)) != 0;
  }
  int key_code() const { return raw_key_code_ & 0x1FF; }
  int raw_key_code() const { return raw_key_code_; }
  bool repeat() const { return repeat_; }

  const bool shift_key() const {
    return (raw_key_code_ & static_cast<int>(Modifier::Shift)) != 0;
  }

  static EventType ConvertToEventType(uint32_t message);
  static int ConvertToKeyCode(WPARAM wParam);
  static bool ConvertToRepeat(LPARAM lParam);

 private:
  int raw_key_code_;
  bool repeat_;
};

//////////////////////////////////////////////////////////////////////
//
// MouseEvent
//
class MouseEvent : public Event {
  DECLARE_CASTABLE_CLASS(MouseEvent, Event);

 public:
  MouseEvent(EventType type,
             MouseButton button,
             int flags,
             int click_count,
             EventTarget* widget,
             const Point& client_point,
             const Point& screen_point);
  MouseEvent(const base::NativeEvent& native_event,
             EventTarget* widget,
             const Point& client_point,
             const Point& screen_point);
  MouseEvent();
  ~MouseEvent() override;

  bool alt_key() const {
    return (flags() & static_cast<int>(EventFlags::AltKey)) != 0;
  }
  MouseButton button() const { return button_; }
  int buttons() const { return buttons_; }
  int click_count() const { return click_count_; }
  bool control_key() const {
    return (flags() & static_cast<int>(EventFlags::ControlKey)) != 0;
  }
  bool is_left_button() const { return button_ == MouseButton::Left; }
  bool is_middle_button() const { return button_ == MouseButton::Middle; }
  bool is_non_client() const {
    return (flags() & static_cast<int>(EventFlags::NonClient)) != 0;
  }
  bool is_right_button() const { return button_ == MouseButton::Right; }
  bool is_other1_button() const { return button_ == MouseButton::Other1; }
  bool is_other2_button() const { return button_ == MouseButton::Other2; }
  Point location() const { return client_point_; }
  Point screen_location() const { return screen_point_; }
  bool shift_key() const {
    return (flags() & static_cast<int>(EventFlags::ShiftKey)) != 0;
  }
  EventTarget* target() const { return target_; }

  static MouseButton ConvertToButton(const base::NativeEvent& native_event);
  static int ConvertToButtons(int flags);
  static EventType ConvertToEventType(const base::NativeEvent& native_event);
  static int ConvertToEventFlags(const base::NativeEvent& native_event);

 private:
  friend class MouseWheelEvent;

  MouseButton button_;
  int buttons_;
  int click_count_;
  Point client_point_;
  Point screen_point_;
  EventTarget* target_;
};

//////////////////////////////////////////////////////////////////////
//
// MouseWheelEvent
//
class MouseWheelEvent final : public MouseEvent {
  DECLARE_CASTABLE_CLASS(MouseEvent, MouseEvent);

 public:
  MouseWheelEvent(EventTarget* widget,
                  const Point& client_point,
                  const Point& screen_point,
                  int flags,
                  int delta);
  ~MouseWheelEvent() final;

  int delta() const { return delta_; }

 private:
  friend class MouseEvent;

  int delta_;
};

}  // namespace ui

std::ostream& operator<<(std::ostream& out, ui::EventType event_type);
std::ostream& operator<<(std::ostream& out, const ui::Event& event);
std::ostream& operator<<(std::ostream& out, const ui::Event* event);
std::ostream& operator<<(std::ostream& out, const ui::KeyEvent& event);
std::ostream& operator<<(std::ostream& out, const ui::KeyEvent* event);
std::ostream& operator<<(std::ostream& out, const ui::MouseEvent& event);
std::ostream& operator<<(std::ostream& out, const ui::MouseEvent* event);
std::ostream& operator<<(std::ostream& out, const ui::MouseWheelEvent& event);
std::ostream& operator<<(std::ostream& out, const ui::MouseWheelEvent* event);

#endif  // EVITA_UI_EVENTS_EVENT_H_
