// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_events_event_target_h)
#define INCLUDE_evita_ui_events_event_target_h

#include "common/castable.h"
#include "evita/ui/events/event_handler.h"

namespace ui {

//////////////////////////////////////////////////////////////////////
//
// EventTarget
//
class EventTarget : public common::Castable, public EventHandler {
  DECLARE_CASTABLE_CLASS(EventTarget, Castable);

  protected: EventTarget();
  public: virtual ~EventTarget();

  DISALLOW_COPY_AND_ASSIGN(EventTarget);
};

}  // namespace ui

#endif // !defined(INCLUDE_evita_ui_events_event_target_h)
