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

class EventInit;
class EventTarget;
class TimeStamp;

namespace bindings {
class EventClass;
}

class Event : public v8_glue::Scriptable<Event> {
  DECLARE_SCRIPTABLE_OBJECT(Event);
  friend class bindings::EventClass;

  public: enum PhaseType {
    kNone,
    kCapturingPhase,
    kAtTarget,
    kBubblingPhase,
  };

  public: class DispatchScope {
    private: Event* event_;

    public: DispatchScope(Event* event, EventTarget* event_target);
    public: ~DispatchScope();

    public: void set_current_target(EventTarget* target);

    public: void StartAtTarget();
    public: void StartBubbling();
  };
  friend class DispatchScope;

  private: bool bubbles_;
  private: bool cancelable_;
  private: gc::Member<EventTarget> current_target_;
  private: bool default_prevented_;
  private: bool dispatched_;
  private: PhaseType event_phase_;
  private: bool stop_immediate_propagation_;
  private: bool stop_propagation_;
  private: gc::Member<EventTarget> target_;
  private: TimeStamp time_stamp_;
  private: base::string16 type_;

  public: Event(const base::string16& type, const EventInit& init_dict);
  private: explicit Event(const base::string16& type);
  public: virtual ~Event();

  public: bool bubbles() const { return bubbles_; }
  public: bool cancelable() const { return cancelable_; }
  public: EventTarget* current_target() const {
    return current_target_.get();
  }
  public: bool default_prevented() const { return default_prevented_; }
  public: bool dispatched() const { return dispatched_; }
  public: PhaseType event_phase() const { return event_phase_; }
  public: EventTarget* target() const { return target_.get(); }
  public: bool stop_immediate_propagation() const {
    return stop_immediate_propagation_;
  }
  public: bool stop_propagation() const { return stop_propagation_; }
  public: TimeStamp time_stamp() const { return time_stamp_; }
  public: const base::string16& type() const { return type_; }

  public: void PreventDefault();
  public: void StopImmediatePropagation();
  public: void StopPropagation();

  DISALLOW_COPY_AND_ASSIGN(Event);
};

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_events_event_h)
