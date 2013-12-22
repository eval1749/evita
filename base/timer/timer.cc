// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "base/timer/timer.h"

#include "base/logging.h"
#include "base/memory/singleton.h"
#include "base/win/native_window.h"
#include "base/win/point.h"
#include "base/win/size.h"
#include <memory>

#define DEBUG_TIMER 0

namespace base {
namespace impl {

struct BASE_EXPORT TimerEntry : RefCounted<TimerEntry> {
  AbstractTimer* timer;
  uint repeat_interval_ms;
  TimerEntry(AbstractTimer* timer, uint repeat_interval_ms)
      : timer(timer), repeat_interval_ms(repeat_interval_ms) {
    #if DEBUG_TIMER
      DEBUG_PRINTF("%p repeat=%d\n", this, repeat_interval_ms);
    #endif
  }
  ~TimerEntry() {
    #if DEBUG_TIMER
      DEBUG_PRINTF("%p repeat=%d\n", this, repeat_interval_ms);
    #endif
  }
};

class TimerController : public Singleton<TimerController> {
  friend class Singleton<TimerController>;

  private: class MessageWindow : public base::win::NativeWindow {
    public: MessageWindow() = default;
    public: virtual ~MessageWindow() = default;
    DISALLOW_COPY_AND_ASSIGN(MessageWindow);
  };

  private: std::unique_ptr<MessageWindow> message_window_;

  private: TimerController()
      : message_window_(CreateMessageWindow()) {
  }

  private: UINT_PTR ComputeCookie(AbstractTimer* timer) {
    auto const entry = timer->entry_.get();
    DCHECK(entry);
    auto const cookie = reinterpret_cast<UINT_PTR>(entry);
    return cookie;
  }

  private: static std::unique_ptr<MessageWindow> CreateMessageWindow() {
    std::unique_ptr<MessageWindow> window(new MessageWindow());
    window->CreateWindowEx(0, 0, nullptr, HWND_MESSAGE,
                           base::win::Point(), base::win::Size());
    return std::move(window);
  }

  private: void SetTimer(AbstractTimer* timer,
                         uint next_fire_interval_ms) {
    ::SetTimer(*message_window_,
               ComputeCookie(timer),
               next_fire_interval_ms,
               TimerController::TimerProc);
  }

  public: void StartTimer(AbstractTimer* timer,
                          uint next_fire_interval_ms,
                          uint repeat_interval_ms) {
    if (!timer->entry_)
      timer->entry_.reset(new TimerEntry(timer, repeat_interval_ms));
    else
      timer->entry_->repeat_interval_ms = repeat_interval_ms;
    timer->entry_->AddRef();
    SetTimer(timer, next_fire_interval_ms);
  }

  public: void StopTimer(AbstractTimer* timer) {
    auto const entry = timer->entry_;
    if (!entry->timer)
      return;
    ::KillTimer(*message_window_, ComputeCookie(timer));
    entry->timer = nullptr;
    entry->Release();
    timer->entry_.reset();
  }

  private: static void CALLBACK TimerProc(HWND, UINT, UINT_PTR cookie,
                                          DWORD) {
    base::scoped_refptr<TimerEntry> entry =
        reinterpret_cast<TimerEntry*>(cookie);
    auto const timer = entry->timer;
    if (!timer) {
      // |timer| is already stopped.
      return;
    }
    timer->Fire();
    if (!entry->timer) {
      // Timer::Fire() destroys |timer|.
      return;
    }
    if (entry->repeat_interval_ms)
      instance().SetTimer(timer, entry->repeat_interval_ms);
    else
      instance().StopTimer(timer);
  }

  DISALLOW_COPY_AND_ASSIGN(TimerController);
};

AbstractTimer::AbstractTimer() {
}

AbstractTimer::~AbstractTimer() {
  if (!entry_)
    return;
  Stop();
}

void AbstractTimer::Start(uint next_fire_interval_ms,
                          uint repeat_interval_ms) {
  TimerController::instance().StartTimer(this, next_fire_interval_ms,
                                         repeat_interval_ms);
}

void AbstractTimer::Stop() {
  TimerController::instance().StopTimer(this);
}

} // namespace impl
} // namespace base
