// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/ui_event.h"

#include "evita/dom/bindings/v8_glue_UiEventInit.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// UiEvent
//
UiEvent::UiEvent(const base::string16& type, const UiEventInit& init_dict)
    : ScriptableBase(type, init_dict),
      detail_(init_dict.detail()),
      view_(init_dict.view()) {}

UiEvent::UiEvent(const base::string16& type) : UiEvent(type, UiEventInit()) {}

UiEvent::~UiEvent() {}

}  // namespace dom
