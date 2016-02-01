// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/event.h"

#include "evita/dom/bindings/v8_glue_EventInit.h"
#include "evita/dom/events/event_target.h"
#include "evita/dom/script_host.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// Event::DispatchScope
//
Event::DispatchScope::DispatchScope(Event* event, EventTarget* target)
    : event_(event) {
  DCHECK_EQ(kNone, event_->event_phase_);
  DCHECK(!event->current_target_);
  DCHECK(!event->dispatched_);
  event_->dispatched_ = true;
  event_->target_ = target;
  event_->event_phase_ = kCapturingPhase;
  event_->time_stamp_ = TimeStamp::Now();
}

Event::DispatchScope::~DispatchScope() {
  event_->current_target_ = nullptr;
  event_->event_phase_ = kNone;
}

void Event::DispatchScope::set_current_target(EventTarget* target) {
  event_->current_target_ = target;
}

void Event::DispatchScope::StartAtTarget() {
  DCHECK_EQ(kCapturingPhase, event_->event_phase_);
  event_->event_phase_ = kAtTarget;
}

void Event::DispatchScope::StartBubbling() {
  DCHECK_EQ(kAtTarget, event_->event_phase_);
  event_->event_phase_ = kBubblingPhase;
}

//////////////////////////////////////////////////////////////////////
//
// Event
//
Event::Event(const base::string16& type, const EventInit& init_dict)
    : bubbles_(init_dict.bubbles()),
      cancelable_(init_dict.cancelable()),
      default_prevented_(false),
      dispatched_(false),
      event_phase_(kNone),
      stop_immediate_propagation_(false),
      stop_propagation_(false),
      type_(type) {}

Event::Event(const base::string16& type) : Event(type, EventInit()) {}

Event::~Event() {}

void Event::PreventDefault() {
  default_prevented_ = cancelable_;
}

void Event::StopImmediatePropagation() {
  stop_immediate_propagation_ = true;
}

void Event::StopPropagation() {
  stop_propagation_ = true;
}

}  // namespace dom
