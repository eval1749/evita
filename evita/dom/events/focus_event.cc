// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/focus_event.h"

#include "evita/bindings/FocusEventInit.h"
#include "evita/dom/converter.h"
#include "evita/dom/events/event_target.h"
#include "evita/v8_glue/optional.h"
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

  private: static FocusEvent* NewFocusEvent(const base::string16& type,
      v8_glue::Optional<FocusEventInit> opt_dict) {
    return new FocusEvent(type, opt_dict.value);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("relatedTarget", &FocusEvent::related_target);
  }

  DISALLOW_COPY_AND_ASSIGN(FocusEventClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// FocusEvent
//
DEFINE_SCRIPTABLE_OBJECT(FocusEvent, FocusEventClass);

FocusEvent::FocusEvent(const base::string16& type,
                       const FocusEventInit& init_dict)
    : ScriptableBase(type, init_dict),
      related_target_(init_dict.related_target()) {
}

FocusEvent::~FocusEvent() {
}

}  // namespace dom
