// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/keyboard_event.h"

#include "evita/bindings/KeyboardEventInit.h"
#include "evita/dom/converter.h"
#include "evita/v8_glue/optional.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {
namespace {
//////////////////////////////////////////////////////////////////////
//
// KeyboardEventClass
//
class KeyboardEventClass :
    public v8_glue::DerivedWrapperInfo<KeyboardEvent, UiEvent> {

  public: explicit KeyboardEventClass(const char* name)
      : BaseClass(name) {
  }
  public: ~KeyboardEventClass() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate, 
        &KeyboardEventClass::NewKeyboardEvent);
  }

  private: static KeyboardEvent* NewKeyboardEvent(const base::string16& type,
      v8_glue::Optional<KeyboardEventInit> opt_dict) {
    return new KeyboardEvent(type, opt_dict.value);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("altKey", &KeyboardEvent::alt_key)
        .SetProperty("ctrlKey", &KeyboardEvent::ctrl_key)
        .SetProperty("keyCode", &KeyboardEvent::key_code)
        .SetProperty("location", &KeyboardEvent::location)
        .SetProperty("metaKey", &KeyboardEvent::meta_key)
        .SetProperty("repeat", &KeyboardEvent::repeat)
        .SetProperty("shiftKey", &KeyboardEvent::shift_key);
  }

  DISALLOW_COPY_AND_ASSIGN(KeyboardEventClass);
};

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
  init_dict.set_repeat(event.repeat);
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
DEFINE_SCRIPTABLE_OBJECT(KeyboardEvent, KeyboardEventClass);

KeyboardEvent::KeyboardEvent(const domapi::KeyboardEvent& event)
    : KeyboardEvent(ConvertEventType(event), ToKeyboardEventInit(event)) {
}

KeyboardEvent::KeyboardEvent(const base::string16& type,
                             const KeyboardEventInit& init_dict)
    : ScriptableBase(type, init_dict),
      alt_key_(init_dict.alt_key()), ctrl_key_(init_dict.ctrl_key()),
      key_code_(init_dict.key_code()), location_(init_dict.location()),
      meta_key_(init_dict.meta_key()), repeat_(init_dict.repeat()),
      shift_key_(init_dict.shift_key()) {
}

KeyboardEvent::~KeyboardEvent() {
}

}  // namespace dom
