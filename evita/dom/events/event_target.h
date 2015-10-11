// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_EVENTS_EVENT_TARGET_H_
#define EVITA_DOM_EVENTS_EVENT_TARGET_H_

#include <vector>

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "evita/gc/member.h"
#include "evita/dom/events/event.h"
#include "evita/dom/public/event_target_id.h"
#include "evita/v8_glue/scriptable.h"

namespace v8_glue {
class Runner;
}

namespace dom {

class Event;

namespace bindings {
class EventTargetClass;
}

typedef v8::Handle<v8::Object> EventListener;

//////////////////////////////////////////////////////////////////////
//
// EventTarget
//
class EventTarget : public v8_glue::Scriptable<EventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(EventTarget);

 public:
  using EventPath = std::vector<EventTarget*>;

  EventTarget();
  ~EventTarget() override;

  virtual bool DispatchEvent(Event* event);

  // TODO(eval1749): We should get rid of
  // |EventTarget::ScheduleDispatchEventDeprecated()|.
  void ScheduleDispatchEventDeprecated(Event* event);

 protected:
  virtual EventPath BuildEventPath() const;

 private:
  friend class bindings::EventTargetClass;
  class EventListenerMap;

  void AddEventListener(const base::string16& type,
                        EventListener callback,
                        bool capture);
  void AddEventListener(const base::string16& type, EventListener callback);
  void DispatchEventWithInLock(Event* event);
  void InvokeEventListeners(v8_glue::Runner* runner, Event* event);
  void RemoveEventListener(const base::string16& type,
                           EventListener callback,
                           bool capture);
  void RemoveEventListener(const base::string16& type, EventListener callback);
  std::unique_ptr<EventListenerMap> event_listener_map_;

  DISALLOW_COPY_AND_ASSIGN(EventTarget);
};

}  // namespace dom

#endif  // EVITA_DOM_EVENTS_EVENT_TARGET_H_
