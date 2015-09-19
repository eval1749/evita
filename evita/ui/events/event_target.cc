// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/events/event_target.h"

#include "evita/ui/events/event_dispatcher.h"

namespace ui {

EventTarget::EventTarget() {}

EventTarget::~EventTarget() {}

bool EventTarget::DispatchEvent(Event* event) {
  return EventDispatcher::instance()->DispatchEvent(this, event);
}

}  // namespace ui
