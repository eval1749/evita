// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/window_event.h"

#include "evita/bindings/EventInit.h"
#include "evita/bindings/WindowEventInit.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// WindowEvent
//
WindowEvent::WindowEvent(const base::string16& type,
                         const WindowEventInit& init_dict)
  : ScriptableBase(type, init_dict),
    source_window_(init_dict.source_window()) {
}

WindowEvent::WindowEvent(const base::string16& type)
    : WindowEvent(type, WindowEventInit()) {
}

WindowEvent::~WindowEvent() {
}

}  // namespace dom
