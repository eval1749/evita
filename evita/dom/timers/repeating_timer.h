// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef EVITA_DOM_TIMERS_REPEATING_TIMER_H_
#define EVITA_DOM_TIMERS_REPEATING_TIMER_H_

#include "evita/dom/timers/timer.h"

namespace dom {

namespace bindings {
class RepeatingTimerClass;
}

class RepeatingTimer final : public v8_glue::Scriptable<RepeatingTimer, Timer> {
  DECLARE_SCRIPTABLE_OBJECT(RepeatingTimer);

 public:
  ~RepeatingTimer() final;

 private:
  friend class bindings::RepeatingTimerClass;

  RepeatingTimer();

  DISALLOW_COPY_AND_ASSIGN(RepeatingTimer);
};

}  // namespace dom

#endif  // EVITA_DOM_TIMERS_REPEATING_TIMER_H_
