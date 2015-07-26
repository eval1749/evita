// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#ifndef COMMON_TIMER_TIMER_H_
#define COMMON_TIMER_TIMER_H_

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "common/common_export.h"

namespace common {
namespace impl {

struct COMMON_EXPORT TimerEntry;

class COMMON_EXPORT AbstractTimer {
 public:
  virtual ~AbstractTimer();

  bool is_active() const { return entry_; }

  void Start(int next_fire_interval_ms, int repeat_interval_ms);
  void Stop();

 protected:
  AbstractTimer();

  virtual void Fire() = 0;

 private:
  friend class TimerController;

  scoped_refptr<TimerEntry> entry_;

  DISALLOW_COPY_AND_ASSIGN(AbstractTimer);
};

template <class Receiver, class TimerClass>
class Timer : public AbstractTimer {
 public:
  typedef void (Receiver::*FiredFunction)(TimerClass*);

 protected:
  Timer(Receiver* receiver, FiredFunction function)
      : receiver_(receiver), function_(function) {}

 private:
  void Fire() override {
    (receiver_->*function_)(static_cast<TimerClass*>(this));
  }

  Receiver* const receiver_;
  FiredFunction const function_;

  DISALLOW_COPY_AND_ASSIGN(Timer);
};

}  // namespace impl

template <class Receiver>
class OneShotTimer : public impl::Timer<Receiver, OneShotTimer<Receiver>> {
 public:
  OneShotTimer(Receiver* receiver, FiredFunction function)
      : Timer(receiver, function) {}

  void Start(int interval_ms) { Timer::Start(interval_ms, 0); }

 private:
  DISALLOW_COPY_AND_ASSIGN(OneShotTimer);
};

template <class Receiver>
class RepeatingTimer : public impl::Timer<Receiver, RepeatingTimer<Receiver>> {
 public:
  RepeatingTimer(Receiver* receiver, FiredFunction function)
      : Timer(receiver, function) {}

  void Start(int interval_ms) { Timer::Start(interval_ms, interval_ms); }

 private:
  DISALLOW_COPY_AND_ASSIGN(RepeatingTimer);
};

}  // namespace common

#endif  // COMMON_TIMER_TIMER_H_
