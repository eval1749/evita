// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_ui_events_event_dispatcher_h)
#define INCLUDE_evita_ui_events_event_dispatcher_h

#include "base/macros.h"
#include "common/memory/singleton.h"

namespace ui {

class Event;
class EventHandler;

class EventDispatcher : public common::Singleton<EventDispatcher> {
  DECLARE_SINGLETON_CLASS(EventDispatcher);

  private: EventHandler* current_handler_;

  private: EventDispatcher();
  public: virtual ~EventDispatcher();

  public: void DidDestroyEventHandler(EventHandler* handler);
  public: bool DispatchEvent(EventHandler* handler, Event* event);

  DISALLOW_COPY_AND_ASSIGN(EventDispatcher);
};

}  // namespace ui

#endif // !defined(INCLUDE_evita_ui_events_event_dispatcher_h)
