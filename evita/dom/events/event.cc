// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/event.h"

#include "evita/dom/converter.h"
#include "evita/dom/events/event_target.h"
#include "evita/dom/script_controller.h"

namespace gin {
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
// Event
//
DEFINE_SCRIPTABLE_OBJECT(Event, EventWrapperInfo);

Event::Event()
    : bubbles_(false), cancelable_(false), default_prevented_(false),
      event_phase_(kNone) {
}

Event::~Event() {
}

void Event::InitEvent(const base::string16& type, bool bubbles,
                      bool cancelable) {
  bubbles_ = bubbles;
  cancelable_ = cancelable;
  type_ = type;
}

}  // namespace dom
