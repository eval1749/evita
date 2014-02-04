// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_keyboard_event_h)
#define INCLUDE_evita_dom_events_keyboard_event_h

#include "evita/dom/events/ui_event.h"
#include "evita/dom/public/api_event.h"

namespace domapi {
struct KeyboardEvent;
}

namespace dom {

class KeyboardEvent : public v8_glue::Scriptable<KeyboardEvent, UiEvent> {
  DECLARE_SCRIPTABLE_OBJECT(KeyboardEvent)

  private: domapi::KeyboardEvent raw_event_;

  public: KeyboardEvent(const base::string16& type);
  public: KeyboardEvent(const domapi::KeyboardEvent& raw_event);
  public: virtual ~KeyboardEvent();

  public: bool alt_key() const;
  public: bool ctrl_key() const;
  public: int key_code() const;
  public: int location() const;
  public: bool meta_key() const;
  public: bool repeat() const;
  public: bool shift_key() const;

  public: void Init(int code);

  DISALLOW_COPY_AND_ASSIGN(KeyboardEvent);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_keyboard_event_h)
