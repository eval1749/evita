// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/event.h"

#include "base/strings/stringprintf.h"
#include "evita/dom/converter.h"
#include "evita/dom/events/event_init.h"
#include "evita/dom/events/event_target.h"
#include "evita/dom/script_host.h"
#include "evita/v8_glue/optional.h"
#include "v8_strings.h"

namespace dom {
namespace {
//////////////////////////////////////////////////////////////////////
//
// EventClass
//
class EventClass : public v8_glue::WrapperInfo {
  public: EventClass(const char* name)
      : v8_glue::WrapperInfo(name) {
  }
  public: ~EventClass() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate, 
        &EventClass::NewEvent);
  }

  private: static Event* NewEvent(const base::string16& type,
      v8_glue::Optional<EventInit> opt_dict) {
    return new Event(type, opt_dict.value);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("bubbles", &Event::bubbles)
        .SetProperty("cancelable", &Event::cancelable)
        .SetProperty("current_target", &Event::current_target)
        .SetProperty("defaultPrevented", &Event::default_prevented)
        .SetProperty("eventPhase", &Event::event_phase)
        .SetProperty("timeStamp", &Event::time_stamp)
        .SetProperty("target", &Event::target)
        .SetProperty("type", &Event::type);
  }

    DISALLOW_COPY_AND_ASSIGN(EventClass);
};
}   // namespace

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
DEFINE_SCRIPTABLE_OBJECT(Event, EventClass);

Event::Event(const base::string16& type, const EventInit& init_dict)
    : bubbles_(init_dict.bubbles()),
      cancelable_(init_dict.cancelable()), default_prevented_(false),
      dispatched_(false), event_phase_(kNone),
      stop_immediate_propagation_(false), stop_propagation_(false),
      type_(type) {
}

Event::~Event() {
}

void Event::PreventDefault() {
  default_prevented_ = cancelable_;
}

}  // namespace dom
