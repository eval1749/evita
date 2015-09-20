// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_EVENTS_EVENT_H_
#define EVITA_DOM_EVENTS_EVENT_H_

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

 public:
  // TODO(eval1749) We should use |enum class| for |PhaseType|.
  enum PhaseType {
    kNone,
    kCapturingPhase,
    kAtTarget,
    kBubblingPhase,
  };

  class DispatchScope final {
   public:
    DispatchScope(Event* event, EventTarget* event_target);
    ~DispatchScope();

    void set_current_target(EventTarget* target);

    void StartAtTarget();
    void StartBubbling();

   private:
    Event* event_;

    DISALLOW_COPY_AND_ASSIGN(DispatchScope);
  };

  Event(const base::string16& type, const EventInit& init_dict);
  ~Event() override;

  bool bubbles() const { return bubbles_; }
  bool cancelable() const { return cancelable_; }
  EventTarget* current_target() const { return current_target_.get(); }
  bool default_prevented() const { return default_prevented_; }
  bool dispatched() const { return dispatched_; }
  PhaseType event_phase() const { return event_phase_; }
  EventTarget* target() const { return target_.get(); }
  bool stop_immediate_propagation() const {
    return stop_immediate_propagation_;
  }
  bool stop_propagation() const { return stop_propagation_; }
  TimeStamp time_stamp() const { return time_stamp_; }
  const base::string16& type() const { return type_; }

  void PreventDefault();
  void StopImmediatePropagation();
  void StopPropagation();

 private:
  friend class bindings::EventClass;
  friend class DispatchScope;

  explicit Event(const base::string16& type);

  bool bubbles_;
  bool cancelable_;
  gc::Member<EventTarget> current_target_;
  bool default_prevented_;
  bool dispatched_;
  PhaseType event_phase_;
  bool stop_immediate_propagation_;
  bool stop_propagation_;
  gc::Member<EventTarget> target_;
  TimeStamp time_stamp_;
  base::string16 type_;

  DISALLOW_COPY_AND_ASSIGN(Event);
};

}  // namespace dom

#endif  // EVITA_DOM_EVENTS_EVENT_H_
