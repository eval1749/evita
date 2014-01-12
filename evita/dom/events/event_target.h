// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_event_target_h)
#define INCLUDE_evita_dom_events_event_target_h

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "evita/gc/member.h"
#include "evita/dom/time_stamp.h"
#include "evita/v8_glue/function_template.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {

class Event;

class EventTarget : public v8_glue::Scriptable<EventTarget> {
  DECLARE_SCRIPTABLE_OBJECT(EventTarget)

  protected: EventTarget();
  protected: virtual ~EventTarget();

  public: void AddEventListener(const base::string16& type,
                                v8::Handle<v8::Function> listener,
                                bool capture);
  public: void DispatchEvent(Event* event);
  public: void RemoveEventListener(const base::string16& type,
                                   v8::Handle<v8::Function> listener,
                                   bool capture);
  DISALLOW_COPY_AND_ASSIGN(EventTarget);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_event_target_h)
