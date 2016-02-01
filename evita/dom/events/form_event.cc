// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/form_event.h"

#include "evita/dom/bindings/v8_glue_EventInit.h"
#include "evita/dom/bindings/v8_glue_FormEventInit.h"
#include "evita/dom/public/view_events.h"

namespace dom {

namespace {

base::string16 ConvertEventType(const domapi::FormEvent& event) {
  if (event.event_type == domapi::EventType::FormChange)
    return L"change";
  if (event.event_type == domapi::EventType::FormClick)
    return L"click";
  NOTREACHED();
  return base::string16();
}

FormEventInit ToFormEventInit(const domapi::FormEvent& event) {
  FormEventInit init_dict;
  init_dict.set_bubbles(true);
  init_dict.set_data(event.data);
  return init_dict;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// FormEvent
//
FormEvent::FormEvent(const domapi::FormEvent& event)
    : FormEvent(ConvertEventType(event), ToFormEventInit(event)) {}

FormEvent::FormEvent(const base::string16& type, const FormEventInit& init_dict)
    : ScriptableBase(type, init_dict), data_(init_dict.data()) {}

FormEvent::FormEvent(const base::string16& type)
    : FormEvent(type, FormEventInit()) {}

FormEvent::~FormEvent() {}

}  // namespace dom
