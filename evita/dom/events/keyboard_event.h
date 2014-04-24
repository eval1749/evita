// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_keyboard_event_h)
#define INCLUDE_evita_dom_events_keyboard_event_h

#include "evita/dom/events/ui_event.h"

namespace domapi {
struct KeyboardEvent;
}

namespace dom {

class KeyboardEventInit;

namespace bindings {
class KeyboardEventClass;
}

class KeyboardEvent : public v8_glue::Scriptable<KeyboardEvent, UiEvent> {
  DECLARE_SCRIPTABLE_OBJECT(KeyboardEvent)
  friend class bindings::KeyboardEventClass;

  private: bool alt_key_;
  private: bool ctrl_key_;
  private: int key_code_;
  private: int location_;
  private: bool meta_key_;
  private: bool repeat_;
  private: bool shift_key_;

  private: KeyboardEvent(const base::string16& type,
                         const KeyboardEventInit& init_dict);
  private: explicit KeyboardEvent(const base::string16& type);
  public: explicit KeyboardEvent(const domapi::KeyboardEvent& raw_event);
  public: virtual ~KeyboardEvent();

  private: bool alt_key() const { return alt_key_; }
  private: bool ctrl_key() const { return ctrl_key_; }
  private: int key_code() const { return key_code_; }
  private: int location() const { return location_; }
  private: bool meta_key() const { return meta_key_; }
  private: bool repeat() const { return repeat_; }
  private: bool shift_key() const { return shift_key_; }

  DISALLOW_COPY_AND_ASSIGN(KeyboardEvent);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_keyboard_event_h)
