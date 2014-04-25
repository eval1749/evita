// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_timers_repeating_timer_h)
#define INCLUDE_evita_dom_timers_repeating_timer_h

#include "evita/dom/timers/timer.h"

namespace dom {

class RepeatingTimer : public v8_glue::Scriptable<RepeatingTimer, Timer> {
  DECLARE_SCRIPTABLE_OBJECT(RepeatingTimer);

  public: RepeatingTimer();
  public: ~RepeatingTimer();

  DISALLOW_COPY_AND_ASSIGN(RepeatingTimer);
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_timers_repeating_timer_h)
