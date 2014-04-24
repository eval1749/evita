// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/mouse_event.h"

#include "evita/bindings/MouseEventInit.h"
#include "evita/dom/converter.h"
#include "evita/dom/events/event_target.h"
#include "evita/dom/public/view_event.h"
#include "evita/v8_glue/optional.h"
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

int ConvertClickCount(const domapi::MouseEvent& event) {
  if (event.event_type == domapi::EventType::Click)
    return 1;
  if (event.event_type == domapi::EventType::DblClick)
    return 2;
  return 0;
}

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

MouseEventInit ToMouseEventInit(const domapi::MouseEvent& event) {
  MouseEventInit init_dict;

  // EventInit
  init_dict.set_bubbles(true);
  init_dict.set_cancelable(true);

  // UiEventInit
  init_dict.set_detail(ConvertClickCount(event));

  // MouseEventInit
  init_dict.set_alt_key(event.alt_key);
  init_dict.set_button(static_cast<int>(event.button));
  init_dict.set_buttons(event.buttons);
  init_dict.set_client_x(event.client_x);
  init_dict.set_client_y(event.client_y);
  init_dict.set_ctrl_key(event.control_key);
  init_dict.set_meta_key(false);
  init_dict.set_shift_key(event.shift_key);

  return init_dict;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// MouseEvent
//
DEFINE_SCRIPTABLE_OBJECT(MouseEvent, MouseEventClass);

MouseEvent::MouseEvent(const domapi::MouseEvent& event)
    : MouseEvent(ConvertEventType(event), ToMouseEventInit(event)) {
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
