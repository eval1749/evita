// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_UI_EVENTS_EVENT_DISPATCHER_H_
#define EVITA_UI_EVENTS_EVENT_DISPATCHER_H_

#include "base/macros.h"
#include "common/memory/singleton.h"

namespace ui {

class Event;
class EventHandler;

class EventDispatcher final : public common::Singleton<EventDispatcher> {
  DECLARE_SINGLETON_CLASS(EventDispatcher);

 public:
  virtual ~EventDispatcher();

  void DidDestroyEventHandler(EventHandler* handler);
  bool DispatchEvent(EventHandler* handler, Event* event);

 private:
  EventDispatcher();

  EventHandler* current_handler_;

  DISALLOW_COPY_AND_ASSIGN(EventDispatcher);
};

}  // namespace ui

#endif  // EVITA_UI_EVENTS_EVENT_DISPATCHER_H_
