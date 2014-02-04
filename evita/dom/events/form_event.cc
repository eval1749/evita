// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/form_event.h"

#include "evita/dom/converter.h"
#include "evita/dom/events/form_event_init_dict.h"
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
      v8_glue::Optional<v8::Handle<v8::Object>> opt_dict) {
    FormEventInitDict init_dict;
    if (!init_dict.Init(opt_dict.value))
      return nullptr;
    return new FormEvent(type, init_dict);
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
                     const FormEventInitDict& init_dict)
    : Event(type, init_dict), data_(init_dict.data()) {
}

FormEvent::~FormEvent() {
}

}  // namespace dom
