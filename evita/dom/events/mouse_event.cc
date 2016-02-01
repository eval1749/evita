// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/mouse_event.h"

#include "evita/dom/bindings/v8_glue_MouseEventInit.h"
#include "evita/dom/public/view_events.h"

namespace dom {
namespace {

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
  if (event.event_type == domapi::EventType::MouseEnter)
    return L"mouseenter";
  if (event.event_type == domapi::EventType::MouseLeave)
    return L"mouseleave";
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
MouseEvent::MouseEvent(const domapi::MouseEvent& event)
    : MouseEvent(ConvertEventType(event), ToMouseEventInit(event)) {}

MouseEvent::MouseEvent(const base::string16& type,
                       const MouseEventInit& init_dict)
    : ScriptableBase(type, init_dict),
      alt_key_(init_dict.alt_key()),
      button_(init_dict.button()),
      buttons_(init_dict.buttons()),
      client_x_(init_dict.client_x()),
      client_y_(init_dict.client_y()),
      ctrl_key_(init_dict.ctrl_key()),
      meta_key_(init_dict.meta_key()),
      shift_key_(init_dict.shift_key()) {}

MouseEvent::MouseEvent(const base::string16& type)
    : MouseEvent(type, MouseEventInit()) {}

MouseEvent::~MouseEvent() {}

}  // namespace dom
