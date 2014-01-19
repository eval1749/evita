// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/form_event.h"

#include "evita/dom/converter.h"
#include "evita/dom/events/event.h"
#include "evita/dom/window.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {
namespace {
//////////////////////////////////////////////////////////////////////
//
// FormEventClass
//
class FormEventClass :
    public v8_glue::DerivedWrapperInfo<FormEvent, Event> {

  public: explicit FormEventClass(const char* name)
      : BaseClass(name) {
  }
  public: ~FormEventClass() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate, 
        &FormEventClass::NewFormEvent);
  }

  private: static FormEvent* NewFormEvent(const base::string16& type,
      Event::BubblingType bubbles, Event::CancelableType cancelable,
      const base::string16& data) {
    return new FormEvent(type, bubbles, cancelable, data);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("data", &FormEvent::data);
  }
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// FormEvent
//
DEFINE_SCRIPTABLE_OBJECT(FormEvent, FormEventClass);

FormEvent::FormEvent(const base::string16& type,
    BubblingType bubbles, CancelableType cancelable,
    const base::string16& data) : data_(data) {
  InitEvent(type, bubbles, cancelable);
}

FormEvent::~FormEvent() {
}

}  // namespace dom
