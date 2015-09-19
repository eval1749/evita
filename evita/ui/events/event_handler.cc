// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/events/event_handler.h"

#include "base/logging.h"
#include "evita/ui/events/event.h"
#include "evita/ui/events/event_dispatcher.h"

namespace ui {

EventHandler::EventHandler() {}

EventHandler::~EventHandler() {
  EventDispatcher::instance()->DidDestroyEventHandler(this);
}

void EventHandler::OnEvent(Event* event) {
  if (auto const key_event = event->as<KeyEvent>()) {
    OnKeyEvent(key_event);
    return;
  }
  if (auto const mouse_event = event->as<MouseEvent>()) {
    OnMouseEvent(mouse_event);
    return;
  }
  NOTREACHED();
}

void EventHandler::OnKeyEvent(KeyEvent* event) {
  DCHECK(event);
}

void EventHandler::OnMouseEvent(MouseEvent* event) {
  DCHECK(event);
}

}  // namespace ui
