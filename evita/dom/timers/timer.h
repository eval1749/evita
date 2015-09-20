// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_TIMERS_TIMER_H_
#define EVITA_DOM_TIMERS_TIMER_H_

#include <memory>

#include "evita/v8_glue/optional.h"
#include "evita/v8_glue/scriptable.h"
#include "evita/v8_glue/scoped_persistent.h"

namespace base {
class Timer;
}

namespace dom {

namespace bindings {
class TimerClass;
}

class Timer : public v8_glue::Scriptable<Timer> {
  DECLARE_SCRIPTABLE_OBJECT(Timer);

 public:
  ~Timer() override;

  bool is_running() const;

  void Reset();

 protected:
  enum class Type {
    OneShot,
    Repeating,
  };

  explicit Timer(Type type);

 private:
  friend class bindings::TimerClass;

  void DidFireTimer();
  void Stop();
  void Start(int delay_ms, v8::Handle<v8::Function> callback);
  void Start(int delay_ms,
             v8::Handle<v8::Function> callback,
             v8::Handle<v8::Value> receiver);

  v8_glue::ScopedPersistent<v8::Function> callback_;
  v8_glue::ScopedPersistent<v8::Value> receiver_;
  std::unique_ptr<base::Timer> timer_;

  DISALLOW_COPY_AND_ASSIGN(Timer);
};

}  // namespace dom

#endif  // EVITA_DOM_TIMERS_TIMER_H_
