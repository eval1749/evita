// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/dom/timers/timer.h"

#include <unordered_map>

#include "base/time/time.h"
#include "base/timer/timer.h"
#include "common/memory/singleton.h"
#include "evita/dom/lock.h"
#include "evita/dom/script_host.h"
#include "evita/v8_glue/runner.h"

namespace dom {

namespace {
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
  auto const runner = ScriptHost::instance()->runner();
  auto const isolate = runner->isolate();
  v8_glue::Runner::Scope runner_scope(runner);
  auto const callback = callback_.NewLocal(isolate);
  auto const receiver = receiver_.NewLocal(isolate);
  DOM_AUTO_LOCK_SCOPE();
  runner->Call(callback, receiver);
}

void Timer::Stop() {
  timer_->Stop();
  TimerList::instance()->Unregister(this);
}

void Timer::Start(int delay_ms, v8::Handle<v8::Function> callback,
                  v8::Handle<v8::Value> receiver) {
  auto const isolate = v8::Isolate::GetCurrent();
  callback_.Reset(isolate, callback);
  receiver_.Reset(isolate, receiver);
  TimerList::instance()->Register(this);
  timer_->Start(FROM_HERE, base::TimeDelta::FromMilliseconds(delay_ms),
                base::Bind(&Timer::DidFireTimer, base::Unretained(this)));
}

void Timer::Start(int delay_ms, v8::Handle<v8::Function> callback) {
  auto const isolate = v8::Isolate::GetCurrent();
  Start(delay_ms, callback, GetWrapper(isolate));
}

}  // namespace dom
