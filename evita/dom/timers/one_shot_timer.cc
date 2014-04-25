// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/timers/one_shot_timer.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// OneShotTimerClass
//
class OneShotTimerClass :
    public v8_glue::DerivedWrapperInfo<OneShotTimer, Timer> {

  public: OneShotTimerClass(const char* name)
      : BaseClass(name) {
  }
  public: ~OneShotTimerClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &OneShotTimerClass::NewOneShotTimer);
  }

  private: static OneShotTimer* NewOneShotTimer() {
    return new OneShotTimer();
  }

  DISALLOW_COPY_AND_ASSIGN(OneShotTimerClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// OneShotTimer
//
DEFINE_SCRIPTABLE_OBJECT(OneShotTimer, OneShotTimerClass)

OneShotTimer::OneShotTimer() : ScriptableBase(Type::OneShot) {
}

OneShotTimer::~OneShotTimer() {
}

}  // namespace dom
