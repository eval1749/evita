// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/window_event.h"

#include "evita/dom/converter.h"
#include "evita/dom/events/window_event_init.h"
#include "evita/dom/windows/window.h"
#include "evita/v8_glue/optional.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {
namespace {
//////////////////////////////////////////////////////////////////////
//
// WindowEventClass
//
class WindowEventClass :
    public v8_glue::DerivedWrapperInfo<WindowEvent, Event> {

  public: explicit WindowEventClass(const char* name)
      : BaseClass(name) {
  }
  public: ~WindowEventClass() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate, 
        &WindowEventClass::NewWindowEvent);
  }

  private: static WindowEvent* NewWindowEvent(const base::string16& type,
      v8_glue::Optional<WindowEventInit> opt_dict) {
    return new WindowEvent(type, opt_dict.value);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("sourceWindow", &WindowEvent::source_window);
  }

  DISALLOW_COPY_AND_ASSIGN(WindowEventClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// WindowEvent
//
DEFINE_SCRIPTABLE_OBJECT(WindowEvent, WindowEventClass);

WindowEvent::WindowEvent(const base::string16& type,
                         const WindowEventInit& init_dict)
  : ScriptableBase(type, init_dict),
    source_window_(init_dict.source_window()) {
}

WindowEvent::~WindowEvent() {
}

}  // namespace dom
