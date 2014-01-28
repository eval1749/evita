// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/mouse_event.h"

#include "evita/dom/converter.h"
#include "evita/dom/events/event_target.h"
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
                                            int detail) {
    return new MouseEvent(type, detail);
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
};

base::string16 ConvertEventType(const domapi::MouseEvent& event) {
  if (event.event_type == domapi::EventType::Click)
    return L"click";
  if (event.event_type == domapi::EventType::DblClick)
    return L"dblclick";
  if (event.event_type == domapi::EventType::MouseDown)
    return L"mousedown";
  if (event.event_type == domapi::EventType::MouseUp)
    return L"mouseup";
  if (event.event_type == domapi::EventType::Wheel)
    return L"wheel";
  return base::string16();
}

int ConvertClickCount(const domapi::MouseEvent& event) {
  if (event.event_type == domapi::EventType::Click)
    return 1;
  if (event.event_type == domapi::EventType::DblClick)
    return 2;
  return 0;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// MouseEvent
//
DEFINE_SCRIPTABLE_OBJECT(MouseEvent, MouseEventClass);

MouseEvent::MouseEvent(const domapi::MouseEvent& api_event)
    : event_(api_event) {
  InitUiEvent(ConvertEventType(api_event), Bubbling, Cancelable, nullptr,
              ConvertClickCount(api_event));
}

MouseEvent::MouseEvent(const base::string16& type, int detail) {
  event_.target_id = kInvalidWindowId;
  event_.event_type = domapi::EventType::Invalid;
  event_.client_x = 0;
  event_.client_y = 0;
  event_.button = domapi::MouseButton::Left;
  event_.buttons = 0;
  event_.alt_key = false;
  event_.control_key = false;
  event_.shift_key = false;
  InitUiEvent(type, Bubbling, Cancelable, nullptr, detail);
}

MouseEvent::~MouseEvent() {
}

bool MouseEvent::alt_key() const {
  return event_.alt_key;
}

int MouseEvent:: button() const {
  return static_cast<int>(event_.button);
}

int MouseEvent:: buttons() const {
  return event_.buttons;
}

int MouseEvent:: client_x() const {
  return event_.client_x;
}

int MouseEvent:: client_y() const {
  return event_.client_y;
}

bool MouseEvent::ctrl_key() const {
  return event_.control_key;
}

bool MouseEvent::shift_key() const {
  return event_.shift_key;
}

}  // namespace dom
