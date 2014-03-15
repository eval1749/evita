// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_keyboard_event_h)
#define INCLUDE_evita_dom_events_keyboard_event_h

#include "evita/dom/events/ui_event.h"
#include "evita/dom/public/view_event.h"

namespace domapi {
struct KeyboardEvent;
}

namespace dom {

class KeyboardEventInit;

class KeyboardEvent : public v8_glue::Scriptable<KeyboardEvent, UiEvent> {
  DECLARE_SCRIPTABLE_OBJECT(KeyboardEvent)

  private: bool alt_key_;
  private: bool ctrl_key_;
  private: int key_code_;
  private: int location_;
  private: bool meta_key_;
  private: bool repeat_;
  private: bool shift_key_;

  public: KeyboardEvent(const base::string16& type,
                        const KeyboardEventInit& init_dict);
  public: KeyboardEvent(const domapi::KeyboardEvent& raw_event);
  public: virtual ~KeyboardEvent();

  public: bool alt_key() const { return alt_key_; }
  public: bool ctrl_key() const { return ctrl_key_; }
  public: int key_code() const { return key_code_; }
  public: int location() const { return location_; }
  public: bool meta_key() const { return meta_key_; }
  public: bool repeat() const { return repeat_; }
  public: bool shift_key() const { return shift_key_; }

  DISALLOW_COPY_AND_ASSIGN(KeyboardEvent);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_keyboard_event_h)
