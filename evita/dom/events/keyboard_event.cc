// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/keyboard_event.h"

#include "evita/dom/converter.h"
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

  private: static KeyboardEvent* NewKeyboardEvent(const base::string16& type) {
    return new KeyboardEvent(type);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("altKey", &KeyboardEvent::alt_key)
        .SetProperty("ctrlKey", &KeyboardEvent::ctrl_key)
        .SetProperty("code", &KeyboardEvent::key_code)
        .SetProperty("location", &KeyboardEvent::location)
        .SetProperty("metaKey", &KeyboardEvent::meta_key)
        .SetProperty("repeat", &KeyboardEvent::repeat)
        .SetProperty("shiftKey", &KeyboardEvent::shift_key)
        .SetMethod("init", &KeyboardEvent::Init);
  }
};

base::string16 ConvertEventType(const domapi::KeyboardEvent& raw_event) {
  if (raw_event.event_type == domapi::EventType::KeyDown)
    return L"keydown";
  if (raw_event.event_type == domapi::EventType::KeyUp)
    return L"keyup";
  return base::string16();
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// KeyboardEvent
//
DEFINE_SCRIPTABLE_OBJECT(KeyboardEvent, KeyboardEventClass);

KeyboardEvent::KeyboardEvent(const base::string16& type) {
  InitEvent(type, Bubbling, Cancelable);
  raw_event_.alt_key = false;
  raw_event_.control_key = false;
  raw_event_.key_code = 0;
  raw_event_.location = 0;
  raw_event_.meta_key = false;
  raw_event_.repeat = 0;
  raw_event_.shift_key = false;
  raw_event_.target_id = kInvalidWindowId;
}

KeyboardEvent::KeyboardEvent(const domapi::KeyboardEvent& raw_event)
    : raw_event_(raw_event) {
  InitEvent(ConvertEventType(raw_event), Bubbling, Cancelable);
}

KeyboardEvent::~KeyboardEvent() {
}

bool KeyboardEvent::alt_key() const {
  return raw_event_.alt_key;
}

bool KeyboardEvent::ctrl_key() const {
  return raw_event_.control_key;
}

int KeyboardEvent::key_code() const {
  return raw_event_.key_code;
}

int KeyboardEvent::location() const {
  return raw_event_.location;
}

bool KeyboardEvent::meta_key() const {
  return raw_event_.meta_key;
}

bool KeyboardEvent::repeat() const {
  return raw_event_.repeat;
}

bool KeyboardEvent::shift_key() const {
  return raw_event_.shift_key;
}

void KeyboardEvent::Init(int code) {
  raw_event_.key_code = code;
}

}  // namespace dom
