// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_ui_events_event_h)
#define INCLUDE_evita_ui_events_event_h

#include <stdint.h>

namespace ui {

enum class EventType {
  Invalid,
  KeyPressed,
  KeyReleased,
  MousePressed,
  MouseMove,
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

}  // namespace ui

#endif //!defined(INCLUDE_evita_ui_events_event_h)
