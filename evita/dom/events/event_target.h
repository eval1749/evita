// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_event_target_h)
#define INCLUDE_evita_dom_events_event_target_h

#include <vector>

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "evita/gc/member.h"
#include "evita/dom/events/event.h"
#include "evita/dom/public/event_target_id.h"
#include "evita/v8_glue/scriptable.h"

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
  friend class bindings::EventTargetClass;

  public: typedef std::vector<EventTarget*> EventPath;
  private: class EventListenerMap;

  private: std::unique_ptr<EventListenerMap> event_listener_map_;

  protected: EventTarget();
  protected: virtual ~EventTarget();

  private: void AddEventListener(const base::string16& type,
                                EventListener callback,
                                bool capture);
  private: void AddEventListener(const base::string16& type,
                                EventListener callback);
  private: virtual EventPath BuildEventPath() const;
  public: virtual bool DispatchEvent(Event* event);
  private: void DispatchEventWithInLock(Event* event);
  private: void InvokeEventListeners(Event* event);
  private: void RemoveEventListener(const base::string16& type,
                                    EventListener callback,
                                    bool capture);
  private: void RemoveEventListener(const base::string16& type,
                                    EventListener callback);
  public: void ScheduleDispatchEvent(Event* event);

  DISALLOW_COPY_AND_ASSIGN(EventTarget);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_event_target_h)
