// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/composition_event.h"

#include "evita/bindings/CompositionEventInit.h"
#include "evita/dom/public/text_composition_event.h"

namespace dom {

namespace {
base::string16 ConvertEventType(const domapi::TextCompositionEvent& event) {
  if (event.event_type == domapi::EventType::TextCompositionCancel)
    return L"compositioncancel";
  if (event.event_type == domapi::EventType::TextCompositionEnd)
    return L"compositionend";
  if (event.event_type == domapi::EventType::TextCompositionStart)
    return L"compositionstart";
  if (event.event_type == domapi::EventType::TextCompositionUpdate)
    return L"compositionupdate";
  NOTREACHED();
  return base::string16();
}

CompositionEventInit ConvertToInitDict(
    const domapi::TextCompositionEvent& event) {
  CompositionEventInit init_dict;

  // EventInit
  init_dict.set_bubbles(true);
  init_dict.set_cancelable(true);

  // UiEventInit
  init_dict.set_detail(0);

  // CompositionEventInit
  init_dict.set_attributes(event.data.attributes);
  init_dict.set_caret(event.data.caret);
  init_dict.set_data(event.data.text);

  return init_dict;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// CompositionEvent
//
CompositionEvent::CompositionEvent(const base::string16& type,
                                   const CompositionEventInit& init_dict)
    : ScriptableBase(type, init_dict),
      attributes_(init_dict.attributes()), caret_(init_dict.caret()),
      data_(init_dict.data()) {
}

CompositionEvent::CompositionEvent(const base::string16& type)
    : CompositionEvent(type, CompositionEventInit()) {
}

CompositionEvent::CompositionEvent(const domapi::TextCompositionEvent& event)
    : CompositionEvent(ConvertEventType(event), ConvertToInitDict(event)) {
}

CompositionEvent::~CompositionEvent() {
}

}  // namespace dom
