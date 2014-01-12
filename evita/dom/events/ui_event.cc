// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/ui_event.h"

#include "evita/dom/converter.h"
#include "evita/dom/window.h"
#include "evita/v8_glue/wrapper_info.h"

namespace dom {
namespace {
//////////////////////////////////////////////////////////////////////
//
// UiEventWrapperInfo
//
class UiEventWrapperInfo :
    public v8_glue::DerivedWrapperInfo<UiEvent, Event> {

  public: explicit UiEventWrapperInfo(const char* name)
      : BaseClass(name) {
  }
  public: ~UiEventWrapperInfo() = default;

  protected: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate, 
        &UiEventWrapperInfo::NewUiEvent);
  }

  private: static UiEvent* NewUiEvent() {
    return new UiEvent();
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("detail", &UiEvent::detail)
        .SetProperty("view", &UiEvent::view)
        .SetMethod("initUiEvent", &UiEvent::InitUiEvent);
  }
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// UiEvent
//
DEFINE_SCRIPTABLE_OBJECT(UiEvent, UiEventWrapperInfo);

UiEvent::UiEvent()
    : detail_(0), view_(nullptr) {
}

UiEvent::~UiEvent() {
}

void UiEvent::InitUiEvent(const base::string16& type, bool bubbles,
                          bool cancelable, const Nullable<Window>& view,
                          int detail) {
  InitEvent(type, bubbles, cancelable);
  detail_ = detail;
  view_ = view;
}

}  // namespace dom
