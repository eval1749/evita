// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/window_event.h"

#include "evita/dom/converter.h"
#include "evita/dom/events/window_event_init_dict.h"
#include "evita/dom/window.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {
namespace {
//////////////////////////////////////////////////////////////////////
//
// WindowEventWrapperInfo
//
class WindowEventWrapperInfo :
    public v8_glue::DerivedWrapperInfo<WindowEvent, Event> {

  public: explicit WindowEventWrapperInfo(const char* name)
      : BaseClass(name) {
  }
  public: ~WindowEventWrapperInfo() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate, 
        &WindowEventWrapperInfo::NewWindowEvent);
  }

  private: static WindowEvent* NewWindowEvent(const base::string16& type,
      v8_glue::Optional<v8::Handle<v8::Object>> opt_dict) {
    WindowEventInitDict init_dict;
    if (!init_dict.Init(opt_dict.value))
      return nullptr;
    return new WindowEvent(type, init_dict);
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("sourceWindow", &WindowEvent::source_window);
  }
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// WindowEvent
//
DEFINE_SCRIPTABLE_OBJECT(WindowEvent, WindowEventWrapperInfo);

WindowEvent::WindowEvent(const base::string16& type,
                         const WindowInitDict& init_dict)
  : Event(type, init_dict),
    source_window_(init_dict.source_window()) {
}

WindowEvent::WindowEvent() {
}

WindowEvent::~WindowEvent() {
}

}  // namespace dom
