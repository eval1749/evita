// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "evita/dom/events/wheel_event.h"

#include "evita/dom/converter.h"
#include "evita/dom/events/event_target.h"
#include "evita/dom/events/wheel_event_init.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {
namespace {
//////////////////////////////////////////////////////////////////////
//
// WheelEventClass
//
class WheelEventClass :
    public v8_glue::DerivedWrapperInfo<WheelEvent, MouseEvent> {

  public: explicit WheelEventClass(const char* name)
      : BaseClass(name) {
  }
  public: ~WheelEventClass() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate, 
        &WheelEventClass::NewWheelEvent);
  }

  private: static WheelEvent* NewWheelEvent(const base::string16& type,
      v8_glue::Optional<WheelEventInit> opt_dict) {
    return new WheelEvent(type, opt_dict.value);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("deltaMode", &WheelEvent::delta_mode)
        .SetProperty("deltaX", &WheelEvent::delta_x)
        .SetProperty("deltaY", &WheelEvent::delta_y)
        .SetProperty("deltaZ", &WheelEvent::delta_z);
  }

  DISALLOW_COPY_AND_ASSIGN(WheelEventClass);
};

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// WheelEvent
//
DEFINE_SCRIPTABLE_OBJECT(WheelEvent, WheelEventClass);

WheelEvent::WheelEvent(const domapi::WheelEvent& event)
    : WheelEvent(L"wheel", WheelEventInit(event)) {
}

WheelEvent::WheelEvent(const base::string16& type,
                       const WheelEventInit& init_dict)
    : ScriptableBase(type, init_dict), delta_mode_(init_dict.delta_mode()),
      delta_x_(init_dict.delta_x()), delta_y_(init_dict.delta_y()),
      delta_z_(init_dict.delta_z()) {
}

WheelEvent::~WheelEvent() {
}

}  // namespace dom
