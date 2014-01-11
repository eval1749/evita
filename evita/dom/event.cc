// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/event.h"

#include "evita/dom/event_target.h"
#include "evita/dom/script_controller.h"
#include "evita/v8_glue/converter.h"

namespace gin {
template<>
struct Converter<dom::TimeStamp> {
  static v8::Handle<v8::Value> ToV8(v8::Isolate* isolate,
                                    const dom::TimeStamp& time_stamp) {
    return gin::ConvertToV8(isolate, time_stamp);
  }
  static bool FromV8(v8::Isolate* isolate, v8::Handle<v8::Value> val,
                     dom::TimeStamp* out) {
    double double_value;
    if (!gin::ConvertFromV8(isolate, val, &double_value))
      return false;
    *out = dom::TimeStamp(double_value);
    return true;
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
  public: EventWrapperInfo() : v8_glue::WrapperInfo("Event") {
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

Event::Event(const base::string16& type, bool bubbles, bool cancelable)
    : bubbles_(bubbles), cancelable_(cancelable), type_(type) {
}

Event::~Event() {
}

v8_glue::WrapperInfo* Event::static_wrapper_info() {
  DEFINE_STATIC_LOCAL(EventWrapperInfo, wrapper_info, ());
  return &wrapper_info;
}

}  // namespace dom
