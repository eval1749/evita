// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_EVENTS_KEYBOARD_EVENT_H_
#define EVITA_DOM_EVENTS_KEYBOARD_EVENT_H_

#include "evita/dom/events/ui_event.h"

namespace domapi {
struct KeyboardEvent;
}

namespace dom {

class KeyboardEventInit;

namespace bindings {
class KeyboardEventClass;
}

class KeyboardEvent final : public ginx::Scriptable<KeyboardEvent, UiEvent> {
  DECLARE_SCRIPTABLE_OBJECT(KeyboardEvent)

 public:
  explicit KeyboardEvent(const domapi::KeyboardEvent& raw_event);
  ~KeyboardEvent() final;

 private:
  friend class bindings::KeyboardEventClass;

  KeyboardEvent(const base::string16& type, const KeyboardEventInit& init_dict);
  explicit KeyboardEvent(const base::string16& type);

  bool alt_key() const { return alt_key_; }
  bool ctrl_key() const { return ctrl_key_; }
  int key_code() const { return key_code_; }
  int location() const { return location_; }
  bool meta_key() const { return meta_key_; }
  bool repeat() const { return repeat_; }
  bool shift_key() const { return shift_key_; }

  bool alt_key_;
  bool ctrl_key_;
  int key_code_;
  int location_;
  bool meta_key_;
  bool repeat_;
  bool shift_key_;

  DISALLOW_COPY_AND_ASSIGN(KeyboardEvent);
};

}  // namespace dom

#endif  // EVITA_DOM_EVENTS_KEYBOARD_EVENT_H_
