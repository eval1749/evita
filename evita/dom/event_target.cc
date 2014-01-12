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
  public: EventTargetWrapperInfo(const char* name)
      : v8_glue::WrapperInfo(name) {
  }
  public: ~EventTargetWrapperInfo() = default;

  private: static EventTarget* NewEventTarget() {
    ScriptController::instance()->ThrowError("Can't create EventTarget.");
    return nullptr;
  }
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// EventTarget
//
DEFINE_SCRIPTABLE_OBJECT(EventTarget, EventTargetWrapperInfo);

EventTarget::EventTarget() {
}

EventTarget::~EventTarget() {
}

}  // namespace dom
