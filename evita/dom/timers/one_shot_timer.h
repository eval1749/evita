// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_TIMERS_ONE_SHOT_TIMER_H_
#define EVITA_DOM_TIMERS_ONE_SHOT_TIMER_H_

#include "evita/dom/timers/timer.h"

namespace dom {

namespace bindings {
class OneShotTimerClass;
}

class OneShotTimer final : public v8_glue::Scriptable<OneShotTimer, Timer> {
  DECLARE_SCRIPTABLE_OBJECT(OneShotTimer);

 public:
  ~OneShotTimer() final;

 private:
  friend class bindings::OneShotTimerClass;

  OneShotTimer();

  DISALLOW_COPY_AND_ASSIGN(OneShotTimer);
};

}  // namespace dom

#endif  // EVITA_DOM_TIMERS_ONE_SHOT_TIMER_H_
