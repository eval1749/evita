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

  private: static OneShotTimer* NewOneShotTimer(int delay_ms,
      v8::Handle<v8::Function> callback) {
    return new OneShotTimer(delay_ms, callback);
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

  private: static RepeatingTimer* NewRepeatingTimer(int delay_ms,
      v8::Handle<v8::Function> callback) {
    return new RepeatingTimer(delay_ms, callback);
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

base::string16 V8ToString(v8::Handle<v8::Value> value) {
  v8::String::Value string_value(value);
  if (!string_value.length())
    return base::string16();
  return base::string16(reinterpret_cast<base::char16*>(*string_value),
                        static_cast<size_t>(string_value.length()));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Timer
//
DEFINE_SCRIPTABLE_OBJECT(Timer, TimerClass)

Timer::Timer(Type type, v8::Handle<v8::Function> callback)
    : callback_(v8::Isolate::GetCurrent(), callback),
      timer_(new base::Timer(type == Type::Repeating,
                             type == Type::Repeating)),
      type_(type) {
}

Timer::~Timer() {
  DCHECK(!timer_->IsRunning());
  TimerList::instance()->Unregister(this);
}

bool Timer::is_running() const {
  return timer_->IsRunning();
}

void Timer::DidFireTimer() {
  auto const isolate = v8::Isolate::GetCurrent();
  v8::HandleScope handle_scope(isolate);
  auto const callback = callback_.NewLocal(isolate);
  if (type_ == Type::OneShot)
    Stop();
  auto const context = ScriptController::instance()->context();
  v8::Context::Scope context_scope(context);
  v8::TryCatch try_catch;
  DOM_AUTO_LOCK_SCOPE();
  callback->Call(v8::Undefined(isolate), 0, nullptr);
  if (try_catch.HasCaught()) {
    DVLOG(0) << "Exception in timer callback " <<
        V8ToString(try_catch.Exception());
  }
}

void Timer::Stop() {
  timer_->Stop();
  TimerList::instance()->Unregister(this);
  callback_.Reset();
}

void Timer::Start(int delay_ms) {
  TimerList::instance()->Register(this);
  timer_->Start(FROM_HERE, base::TimeDelta::FromMilliseconds(delay_ms),
                base::Bind(&Timer::DidFireTimer, base::Unretained(this)));
}

//////////////////////////////////////////////////////////////////////
//
// OneShotTimer
//
DEFINE_SCRIPTABLE_OBJECT(OneShotTimer, OneShotTimerClass)

OneShotTimer::OneShotTimer(int dealy_ms, v8::Handle<v8::Function> callback)
    : ScriptableBase(Type::OneShot, callback) {
  Start(dealy_ms);
}

OneShotTimer::~OneShotTimer() {
}

//////////////////////////////////////////////////////////////////////
//
// RepeatingTimer
//
DEFINE_SCRIPTABLE_OBJECT(RepeatingTimer, RepeatingTimerClass)

RepeatingTimer::RepeatingTimer(int dealy_ms, v8::Handle<v8::Function> callback)
    : ScriptableBase(Type::Repeating, callback) {
  Start(dealy_ms);
}

RepeatingTimer::~RepeatingTimer() {
}

}  // namespace dom
