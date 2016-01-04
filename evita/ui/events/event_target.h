// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_EVENTS_EVENT_TARGET_H_
#define EVITA_UI_EVENTS_EVENT_TARGET_H_

#include "common/castable.h"
#include "evita/ui/events/event_handler.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// EventTarget
//
class EventTarget : public common::Castable<EventTarget>, public EventHandler {
  DECLARE_CASTABLE_CLASS(EventTarget, Castable);

 public:
  ~EventTarget() override;

  bool DispatchEvent(Event* event);

 protected:
  EventTarget();

 private:
  DISALLOW_COPY_AND_ASSIGN(EventTarget);
};

}  // namespace ui

#endif  // EVITA_UI_EVENTS_EVENT_TARGET_H_
