// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/timers/repeating_timer.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// RepeatingTimerClass
//
class RepeatingTimerClass :
    public v8_glue::DerivedWrapperInfo<RepeatingTimer, Timer> {

  public: RepeatingTimerClass(const char* name)
      : BaseClass(name) {
  }
  public: ~RepeatingTimerClass() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    return v8_glue::CreateConstructorTemplate(isolate,
        &RepeatingTimerClass::NewRepeatingTimer);
  }

  private: static RepeatingTimer* NewRepeatingTimer() {
    return new RepeatingTimer();
  }

  DISALLOW_COPY_AND_ASSIGN(RepeatingTimerClass);
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// RepeatingTimer
//
DEFINE_SCRIPTABLE_OBJECT(RepeatingTimer, RepeatingTimerClass)

RepeatingTimer::RepeatingTimer() : ScriptableBase(Type::Repeating) {
}

RepeatingTimer::~RepeatingTimer() {
}

}  // namespace dom
