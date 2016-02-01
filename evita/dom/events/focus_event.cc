// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/focus_event.h"

#include "evita/dom/bindings/v8_glue_FocusEventInit.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// FocusEvent
//
FocusEvent::FocusEvent(const base::string16& type,
                       const FocusEventInit& init_dict)
    : ScriptableBase(type, init_dict),
      related_target_(init_dict.related_target()) {}

FocusEvent::FocusEvent(const base::string16& type)
    : FocusEvent(type, FocusEventInit()) {}

FocusEvent::~FocusEvent() {}

}  // namespace dom
