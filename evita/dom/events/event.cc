// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/event.h"

#include "evita/dom/converter.h"
#include "evita/dom/events/event_target.h"
#include "evita/dom/script_controller.h"

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

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("bubbles", &Event::bubbles)
        .SetProperty("cancelable", &Event::cancelable)
        .SetProperty("currentTarget", &Event::current_target)
        .SetProperty("default_prevented", &Event::default_prevented)
        .SetProperty("timeStamp", &Event::time_stamp)
        .SetProperty("target", &Event::target);
  }
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Event
//
DEFINE_SCRIPTABLE_OBJECT(Event, EventWrapperInfo);

Event::Event(const base::string16& type, bool bubbles, bool cancelable)
    : bubbles_(bubbles), cancelable_(cancelable), type_(type) {
}

Event::~Event() {
}

}  // namespace dom
