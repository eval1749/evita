// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_events_event_h)
#define INCLUDE_evita_dom_events_event_h

#include "base/basictypes.h"
#include "base/strings/string16.h"
#include "evita/gc/member.h"
#include "evita/dom/time_stamp.h"
#include "evita/v8_glue/scriptable.h"

namespace dom {

class EventTarget;
class TimeStamp;

class Event : public v8_glue::Scriptable<Event> {
    DECLARE_SCRIPTABLE_OBJECT(Event);

  private: bool bubbles_;
  private: bool cancelable_;
  private: gc::Member<EventTarget> current_target_;
  private: bool default_prevented_;
  private: gc::Member<EventTarget> target_;
  private: TimeStamp time_stamp_;
  private: base::string16 type_;

  public: Event(const base::string16& type, bool bubbles, bool cancelable);
  public: virtual ~Event();

  public: bool bubbles() const { return bubbles_; }
  public: bool cancelable() const { return cancelable_; }
  public: EventTarget* current_target() const { return current_target_.get(); }
  public: bool default_prevented() const { return default_prevented_; }
  public: EventTarget* target() const { return target_.get(); }
  public: TimeStamp time_stamp() const { return time_stamp_; }
  public: const base::string16& type() const { return type_; }

  DISALLOW_COPY_AND_ASSIGN(Event);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_event_h)
