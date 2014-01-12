// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/events/ui_event.h"

#include "evita/dom/converter.h"
#include "evita/dom/window.h"

namespace dom {
namespace {
//////////////////////////////////////////////////////////////////////
//
// UiEventWrapperInfo
//
class UiEventWrapperInfo : public v8_glue::WrapperInfo {
  public: UiEventWrapperInfo(const char* name)
      : v8_glue::WrapperInfo(name) {
  }
  public: ~UiEventWrapperInfo() = default;

  private: virtual v8_glue::WrapperInfo* inherit_from() const override {
    return Event::static_wrapper_info();
  }

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
