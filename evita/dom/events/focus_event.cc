// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/focus_event.h"

#include "evita/dom/converter.h"
#include "evita/dom/events/event_target.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {
namespace {
//////////////////////////////////////////////////////////////////////
//
// FocusEventClass
//
class FocusEventClass :
    public v8_glue::DerivedWrapperInfo<FocusEvent, UiEvent> {

  public: explicit FocusEventClass(const char* name)
      : BaseClass(name) {
  }
  public: ~FocusEventClass() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate, 
        &FocusEventClass::NewFocusEvent);
  }

  private: static FocusEvent* NewFocusEvent() {
    return new FocusEvent(base::string16(), nullptr);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("relatedTarget", &FocusEvent::related_target);
  }
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// FocusEvent
//
DEFINE_SCRIPTABLE_OBJECT(FocusEvent, FocusEventClass);

FocusEvent::FocusEvent(const base::string16& name,
                       EventTarget* related_target)
    : related_target_(related_target) {
  InitEvent(name, Bubbling, NotCancelable);
}

FocusEvent::~FocusEvent() {
}

}  // namespace dom
