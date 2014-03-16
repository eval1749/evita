// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/mouse_event.h"

#include "evita/dom/converter.h"
#include "evita/dom/events/event_target.h"
#include "evita/dom/events/mouse_event_init.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {
namespace {
//////////////////////////////////////////////////////////////////////
//
// MouseEventClass
//
class MouseEventClass :
    public v8_glue::DerivedWrapperInfo<MouseEvent, UiEvent> {

  public: explicit MouseEventClass(const char* name)
      : BaseClass(name) {
  }
  public: ~MouseEventClass() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate, 
        &MouseEventClass::NewMouseEvent);
  }

  private: static MouseEvent* NewMouseEvent(const base::string16& type,
      v8_glue::Optional<MouseEventInit> opt_dict) {
    return new MouseEvent(type, opt_dict.value);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("altKey", &MouseEvent::alt_key)
        .SetProperty("button", &MouseEvent::button)
        .SetProperty("buttons", &MouseEvent::button)
        .SetProperty("clientX", &MouseEvent::client_x)
        .SetProperty("clientY", &MouseEvent::client_y)
        .SetProperty("ctrlKey", &MouseEvent::ctrl_key)
        .SetProperty("shiftKey", &MouseEvent::shift_key);
  }

  DISALLOW_COPY_AND_ASSIGN(MouseEventClass);
};

base::string16 ConvertEventType(const domapi::MouseEvent& event) {
  if (event.event_type == domapi::EventType::Click)
    return L"click";
  if (event.event_type == domapi::EventType::DblClick)
    return L"dblclick";
  if (event.event_type == domapi::EventType::MouseDown)
    return L"mousedown";
  if (event.event_type == domapi::EventType::MouseMove)
    return L"mousemove";
  if (event.event_type == domapi::EventType::MouseUp)
    return L"mouseup";
  if (event.event_type == domapi::EventType::Wheel)
    return L"wheel";
  return base::string16();
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// MouseEvent
//
DEFINE_SCRIPTABLE_OBJECT(MouseEvent, MouseEventClass);

MouseEvent::MouseEvent(const domapi::MouseEvent& event)
    : MouseEvent(ConvertEventType(event), MouseEventInit(event)) {
}

MouseEvent::MouseEvent(const base::string16& type,
                       const MouseEventInit& init_dict)
    : ScriptableBase(type, init_dict),
      alt_key_(init_dict.alt_key()), button_(init_dict.button()),
      buttons_(init_dict.buttons()), client_x_(init_dict.client_x()),
      client_y_(init_dict.client_y()), ctrl_key_(init_dict.ctrl_key()),
      meta_key_(init_dict.meta_key()), shift_key_(init_dict.shift_key()) {
}

MouseEvent::~MouseEvent() {
}

}  // namespace dom
