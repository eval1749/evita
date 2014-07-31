// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_ui_events_event_h)
#define INCLUDE_evita_ui_events_event_h

#include <stdint.h>

#include "base/time/time.h"
#include "common/win/rect.h"

namespace ui {

using common::win::Point;
using common::win::Rect;
class Widget;

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

//////////////////////////////////////////////////////////////////////
//
// Event
//
class Event {
  private: EventType event_type_;
  private: base::Time time_stamp_;

  protected: explicit Event(EventType event_type);
  protected: Event();
  public: ~Event();

  public: EventType event_type() const { return event_type_; }
  public: base::Time time_stamp() const { return time_stamp_; }
};

//////////////////////////////////////////////////////////////////////
//
// KeyboardEvent
//
class KeyboardEvent : public Event {
  private: int raw_key_code_;
  private: bool repeat_;

  public: KeyboardEvent(EventType type, int key_code, bool repeat);
  public: KeyboardEvent();
  public: ~KeyboardEvent();

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
  friend class MouseWheelEvent;

  public: enum Button {
    kNone = 0,
    kLeft = 0,
    kMiddle = 1,
    kRight = 2,
    kOther1 = 3,
    kOther2 = 4,
  };

  private: bool alt_key_;
  private: int button_;
  private: int buttons_;
  private: int click_count_;
  private: Point client_point_;
  private: bool control_key_;
  private: Point screen_point_;
  private: bool shift_key_;
  private: Widget* target_;

  public: MouseEvent(EventType type, Button button, uint32_t flags,
                     int click_count, Widget* widget,
                     const Point& client_point, const Point& screen_point);
  public: MouseEvent();
  public: ~MouseEvent();

  public: bool alt_key() const { return alt_key_; }
  public: int button() const { return button_; }
  public: int buttons() const { return buttons_; }
  public: int click_count() const { return click_count_; }
  public: bool control_key() const { return control_key_; }
  public: bool is_left_button() const { return button_ == kLeft; }
  public: bool is_middle_button() const { return button_ == kMiddle; }
  public: bool is_right_button() const { return button_ == kRight; }
  public: bool is_other1_button() const { return button_ == kOther1; }
  public: bool is_other2_button() const { return button_ == kOther2; }
  public: Point location() const { return client_point_; }
  public: Point screen_location() const { return screen_point_; }
  public: bool shift_key() const { return shift_key_; }
  public: Widget* target() const { return target_; }

  public: static Button ConvertToButton(uint32_t message, WPARAM wParam);
  private: static int ConvertToButtons(uint32_t flags);
  public: static EventType ConvertToEventType(uint32_t message);
};

//////////////////////////////////////////////////////////////////////
//
// MouseWheelEvent
//
class MouseWheelEvent : public MouseEvent {
  friend class MouseEvent;

  private: int delta_;

  public: MouseWheelEvent(Widget* widget, const Point& client_point,
                          const Point& screen_point, uint32_t flags,
                          int delta);
  public: ~MouseWheelEvent();

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
