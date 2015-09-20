// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/keyboard_event.h"

#include "evita/bindings/v8_glue_KeyboardEventInit.h"
#include "evita/dom/public/view_event.h"

namespace dom {
namespace {
base::string16 ConvertEventType(const domapi::KeyboardEvent& raw_event) {
  if (raw_event.event_type == domapi::EventType::KeyDown)
    return L"keydown";
  if (raw_event.event_type == domapi::EventType::KeyUp)
    return L"keyup";
  return base::string16();
}

KeyboardEventInit ToKeyboardEventInit(const domapi::KeyboardEvent& event) {
  KeyboardEventInit init_dict;
  init_dict.set_alt_key(event.alt_key);
  init_dict.set_ctrl_key(event.control_key);
  init_dict.set_key_code(event.key_code);
  init_dict.set_location(event.location);
  init_dict.set_meta_key(false);
  init_dict.set_repeat(event.repeat != 0);
  init_dict.set_shift_key(event.shift_key);
  init_dict.set_bubbles(true);
  init_dict.set_cancelable(true);
  return init_dict;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// KeyboardEvent
//
KeyboardEvent::KeyboardEvent(const domapi::KeyboardEvent& event)
    : KeyboardEvent(ConvertEventType(event), ToKeyboardEventInit(event)) {}

KeyboardEvent::KeyboardEvent(const base::string16& type,
                             const KeyboardEventInit& init_dict)
    : ScriptableBase(type, init_dict),
      alt_key_(init_dict.alt_key()),
      ctrl_key_(init_dict.ctrl_key()),
      key_code_(init_dict.key_code()),
      location_(init_dict.location()),
      meta_key_(init_dict.meta_key()),
      repeat_(init_dict.repeat()),
      shift_key_(init_dict.shift_key()) {}

KeyboardEvent::KeyboardEvent(const base::string16& type)
    : KeyboardEvent(type, KeyboardEventInit()) {}

KeyboardEvent::~KeyboardEvent() {}

}  // namespace dom
