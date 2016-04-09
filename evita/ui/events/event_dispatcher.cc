// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/events/event_dispatcher.h"

#include "base/logging.h"
#include "evita/ui/events/event.h"
#include "evita/ui/events/event_handler.h"

namespace ui {

EventDispatcher::EventDispatcher() {}

EventDispatcher::~EventDispatcher() {}

void EventDispatcher::DidDestroyEventHandler(EventHandler* handler) {}

bool EventDispatcher::DispatchEvent(EventHandler* handler, Event* event) {
  handler->OnEvent(event);
  return !event->default_prevented();
}

}  // namespace ui
