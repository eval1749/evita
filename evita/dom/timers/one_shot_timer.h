// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_timers_one_shot_timer_h)
#define INCLUDE_evita_dom_timers_one_shot_timer_h

#include "evita/dom/timers/timer.h"

namespace dom {

namespace bindings {
class OneShotTimerClass;
}

class OneShotTimer : public v8_glue::Scriptable<OneShotTimer, Timer> {
  DECLARE_SCRIPTABLE_OBJECT(OneShotTimer);
  friend class bindings::OneShotTimerClass;

  private: OneShotTimer();
  public: ~OneShotTimer();

  DISALLOW_COPY_AND_ASSIGN(OneShotTimer);
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_timers_one_shot_timer_h)
