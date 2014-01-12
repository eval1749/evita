// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/event_target.h"

#include "evita/dom/script_controller.h"
#include "evita/v8_glue/converter.h"

namespace dom {
namespace {
//////////////////////////////////////////////////////////////////////
//
// EventTargetWrapperInfo
//
class EventTargetWrapperInfo : public v8_glue::WrapperInfo {
  public: EventTargetWrapperInfo() : v8_glue::WrapperInfo("EventTarget") {
  }
  public: ~EventTargetWrapperInfo() = default;

  private: static EventTarget* NewEventTarget() {
    ScriptController::instance()->ThrowError("Can't create EventTarget.");
    return nullptr;
  }
};
}  // namespace

EventTarget::EventTarget() {
}

EventTarget::~EventTarget() {
}

v8_glue::WrapperInfo* EventTarget::static_wrapper_info() {
  DEFINE_STATIC_LOCAL(EventTargetWrapperInfo, wrapper_info, ());
  return &wrapper_info;
}

}  // namespace dom
