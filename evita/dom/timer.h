// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_dom_timer_h)
#define INCLUDE_evita_dom_timer_h

#include <memory>

#include "evita/v8_glue/optional.h"
#include "evita/v8_glue/scriptable.h"
#include "evita/v8_glue/scoped_persistent.h"

namespace base {
class Timer;
}

namespace dom {

class Timer : public v8_glue::Scriptable<Timer> {
  DECLARE_SCRIPTABLE_OBJECT(Timer);

  protected: enum class Type {
    OneShot,
    Repeating,
  };

  private: v8_glue::ScopedPersistent<v8::Function> callback_;
  private: v8_glue::ScopedPersistent<v8::Value> receiver_;
  private: std::unique_ptr<base::Timer> timer_;

  protected: Timer(Type type);
  public: ~Timer();

  public: bool is_running() const;

  private: void DidFireTimer();

  public: void Reset();
  public: void Stop();
  public: void Start(int delay_ms, v8::Handle<v8::Function> callback,
                     v8_glue::Optional<v8::Handle<v8::Value>> receiver);

  DISALLOW_COPY_AND_ASSIGN(Timer);
};

class OneShotTimer : public v8_glue::Scriptable<OneShotTimer, Timer> {
  DECLARE_SCRIPTABLE_OBJECT(OneShotTimer);

  public: OneShotTimer();
  public: ~OneShotTimer();
};

class RepeatingTimer : public v8_glue::Scriptable<RepeatingTimer, Timer> {
  DECLARE_SCRIPTABLE_OBJECT(RepeatingTimer);

  public: RepeatingTimer();
  public: ~RepeatingTimer();
};

} // namespace dom

#endif //!defined(INCLUDE_evita_dom_timer_h)
