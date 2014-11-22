// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/events/event_dispatcher.h"

#include "base/logging.h"
#include "evita/ui/events/event.h"
#include "evita/ui/events/event_handler.h"

namespace ui {

EventDispatcher::EventDispatcher() : current_handler_(nullptr) {
}

EventDispatcher::~EventDispatcher() {
  DCHECK(!current_handler_);
}

void EventDispatcher::DidDestroyEventHandler(EventHandler* handler) {
  if (current_handler_ != handler)
    return;
  current_handler_ = nullptr;
}

bool EventDispatcher::DispatchEvent(EventHandler* handler, Event* event) {
  DCHECK(!current_handler_);
  current_handler_ = handler;
  handler->OnEvent(event);
  if (!current_handler_)
    return false;
  current_handler_ = nullptr;
  return !event->default_prevented();
}

}  // namespace ui
