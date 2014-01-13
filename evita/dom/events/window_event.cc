// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/window_event.h"

#include "evita/dom/converter.h"
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

  private: static WindowEvent* NewWindowEvent() {
    return new WindowEvent();
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("sourceWindow", &WindowEvent::source_window)
        .SetMethod("initWindowEvent", &WindowEvent::InitWindowEvent);
  }
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// WindowEvent
//
DEFINE_SCRIPTABLE_OBJECT(WindowEvent, WindowEventWrapperInfo);

WindowEvent::WindowEvent(const base::string16& type, BubblingType bubbles,
                         CancelableType cancelable, Window* source_window) {
  InitWindowEvent(type, bubbles, cancelable, source_window);
}

WindowEvent::WindowEvent() {
}

WindowEvent::~WindowEvent() {
}

void WindowEvent::InitWindowEvent(const base::string16& type,
                                  BubblingType bubbles,
                                  CancelableType cancelable,
                                  Window* source_window) {
  InitEvent(type, bubbles, cancelable);
  source_window_ = source_window;
}

}  // namespace dom
