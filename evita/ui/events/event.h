// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_ui_events_event_h)
#define INCLUDE_evita_ui_events_event_h

#include <stdint.h>

namespace ui {

enum class EventType {
  Invalid,
  KeyDown,
  KeyUp,
  MouseDown,
  MouseMove,
  MouseUp,
  MouseWheel,
};

class Event {
  private: EventType event_type_;

  protected: explicit Event(EventType event_type);
  protected: Event();
  public: ~Event();

  public: EventType event_type() const { return event_type_; }
};

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
