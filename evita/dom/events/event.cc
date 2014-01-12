// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/event.h"

#include "evita/dom/converter.h"
#include "evita/dom/events/event_target.h"
#include "evita/dom/script_controller.h"

namespace gin {
bool Converter<dom::Event::BubblingType>::FromV8(
    v8::Isolate* isolate, v8::Handle<v8::Value> value,
    dom::Event::BubblingType* bubbles) {
  bool bool_value = false;
  if (!ConvertFromV8(isolate, value, &bool_value))
    return false;
  *bubbles = static_cast<dom::Event::BubblingType>(bool_value);
  return true;
}

bool Converter<dom::Event::CancelableType>::FromV8(
    v8::Isolate* isolate, v8::Handle<v8::Value> value,
    dom::Event::CancelableType* Cancels) {
  bool bool_value = false;
  if (!ConvertFromV8(isolate, value, &bool_value))
    return false;
  *Cancels = static_cast<dom::Event::CancelableType>(bool_value);
  return true;
}

template<>
struct Converter<dom::Event::PhaseType> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    dom::Event::PhaseType event_phase) {
    return ConvertToV8(isolate, static_cast<int>(event_phase));
  }
};
}  // namespace gin

namespace dom {
namespace {
//////////////////////////////////////////////////////////////////////
//
// EventWrapperInfo
//
class EventWrapperInfo : public v8_glue::WrapperInfo {
  public: EventWrapperInfo(const char* name)
      : v8_glue::WrapperInfo(name) {
  }
  public: ~EventWrapperInfo() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate, 
        &EventWrapperInfo::NewEvent);
  }

  private: static Event* NewEvent() {
    return new Event();
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
        .SetProperty("type", &Event::type)
        .SetMethod("initEvent", &Event::InitEvent);
  }
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Event::DispatchScope
//
Event::DispatchScope::DispatchScope(Event* event, EventTarget* target)
    : event_(event) {
  DCHECK_EQ(kNone, event_->event_phase_);
  DCHECK(!event->current_target_);
  DCHECK(!event->target_);
  event_->target_ = target;
  event_->event_phase_ = kCapturingPhase;
}

Event::DispatchScope::~DispatchScope() {
  event_->current_target_ = nullptr;
  event_->event_phase_ = kNone;
  event_->target_ = nullptr;
  event_->time_stamp_ = TimeStamp::Now();
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
DEFINE_SCRIPTABLE_OBJECT(Event, EventWrapperInfo);

Event::Event()
    : bubbles_(false), cancelable_(false), default_prevented_(false),
      event_phase_(kNone), stop_immediate_propagation_(false),
      stop_propagation_(false) {
}

Event::~Event() {
}

void Event::InitEvent(const base::string16& type, BubblingType bubbles,
                      CancelableType cancelable) {
  if (event_phase_ != kNone)
    return;
  DCHECK(!current_target_);
  DCHECK(!target_);
  bubbles_ = bubbles;
  cancelable_ = cancelable;
  default_prevented_ = false;
  stop_immediate_propagation_ = false;
  stop_propagation_ = false;
  time_stamp_ = TimeStamp();
  type_ = type;
}

void Event::PreventDefault() {
  default_prevented_ = cancelable_;
}

}  // namespace dom
