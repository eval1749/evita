// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/timer.h"

#include <unordered_map>

#include "base/time/time.h"
#include "base/timer/timer.h"
#include "common/memory/singleton.h"
#include "evita/dom/lock.h"
#include "evita/dom/script_controller.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/function_template_builder.h"

namespace dom {

namespace {
//////////////////////////////////////////////////////////////////////
//
// TimerClass
//
class TimerClass : public v8_glue::WrapperInfo {
  public: TimerClass(const char* name)
      : v8_glue::WrapperInfo(name) {
  }
  public: ~TimerClass() = default;

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("isRunning", &Timer::is_running)
        .SetMethod("start", &Timer::Start)
        .SetMethod("stop", &Timer::Stop);
  }

  DISALLOW_COPY_AND_ASSIGN(TimerClass);
};

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

//////////////////////////////////////////////////////////////////////
//
// TimerList
// TimerList holds timer objects during they are running.
//
class TimerList : public common::Singleton<TimerList> {
  friend class common::Singleton<TimerList>;

  private: typedef v8_glue::ScopedPersistent<v8::Object> TimerHolder;
  private: std::unordered_map<Timer*, TimerHolder*> timers_;

  private: TimerList() = default;
  public: ~TimerList() = default;

  public: void Register(Timer* timer) {
    auto const isolate = v8::Isolate::GetCurrent();
    timers_[timer] = new TimerHolder(isolate, timer->GetWrapper(isolate));
  }

  public: void Unregister(Timer* timer) {
    auto const it = timers_.find(timer);
    if (it == timers_.end())
      return;
    delete it->second;
    timers_.erase(it);
  }
};

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Timer
//
DEFINE_SCRIPTABLE_OBJECT(Timer, TimerClass)

Timer::Timer(Type type)
    : timer_(new base::Timer(type == Type::Repeating,
                             type == Type::Repeating)) {
}

Timer::~Timer() {
  Stop();
}

bool Timer::is_running() const {
  return timer_->IsRunning();
}

void Timer::DidFireTimer() {
  auto const isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  auto const callback = callback_.NewLocal(isolate);
  auto const context = ScriptController::instance()->context();
  v8::Context::Scope context_scope(context);
  v8::TryCatch try_catch;
  try_catch.SetVerbose(true);
  try_catch.SetCaptureMessage(true);
  DOM_AUTO_LOCK_SCOPE();
  callback->Call(receiver_.NewLocal(isolate), 0, nullptr);
  if (try_catch.HasCaught())
    ScriptController::instance()->LogException(try_catch);
}

void Timer::Stop() {
  timer_->Stop();
  TimerList::instance()->Unregister(this);
}

void Timer::Start(int delay_ms, v8::Handle<v8::Function> callback,
                  v8_glue::Optional<v8::Handle<v8::Value>> opt_receiver) {
  auto const isolate = v8::Isolate::GetCurrent();
  callback_.Reset(isolate, callback);
  receiver_.Reset(isolate, opt_receiver.is_supplied ? opt_receiver.value :
                                                      GetWrapper(isolate));
  TimerList::instance()->Register(this);
  timer_->Start(FROM_HERE, base::TimeDelta::FromMilliseconds(delay_ms),
                base::Bind(&Timer::DidFireTimer, base::Unretained(this)));
}

//////////////////////////////////////////////////////////////////////
//
// OneShotTimer
//
DEFINE_SCRIPTABLE_OBJECT(OneShotTimer, OneShotTimerClass)

OneShotTimer::OneShotTimer() : ScriptableBase(Type::OneShot) {
}

OneShotTimer::~OneShotTimer() {
}

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
