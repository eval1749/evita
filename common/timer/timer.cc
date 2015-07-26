// Copyright (C) 1996-2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "common/timer/timer.h"

#include <memory>

#include "base/logging.h"
#include "common/memory/singleton.h"
#include "common/win/native_window.h"
#include "common/win/point.h"
#include "common/win/size.h"

namespace common {
namespace impl {

struct COMMON_EXPORT TimerEntry final : base::RefCounted<TimerEntry> {
  AbstractTimer* timer;
  int repeat_interval_ms;
  TimerEntry(AbstractTimer* timer, int repeat_interval_ms)
      : timer(timer), repeat_interval_ms(repeat_interval_ms) {}
  ~TimerEntry() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(TimerEntry);
};

class TimerController final : public common::Singleton<TimerController> {
 public:
  void StartTimer(AbstractTimer* timer,
                  int next_fire_interval_ms,
                  int repeat_interval_ms) {
    if (!timer->entry_)
      timer->entry_ = new TimerEntry(timer, repeat_interval_ms);
    else
      timer->entry_->repeat_interval_ms = repeat_interval_ms;
    timer->entry_->AddRef();
    SetTimer(timer, next_fire_interval_ms);
  }

  void StopTimer(AbstractTimer* timer) {
    auto const entry = timer->entry_;
    if (!entry->timer)
      return;
    ::KillTimer(*message_window_, ComputeCookie(timer));
    entry->timer = nullptr;
    entry->Release();
    timer->entry_ = nullptr;
  }

 private:
  friend class common::Singleton<TimerController>;

  class MessageWindow final : public common::win::NativeWindow {
   public:
    MessageWindow() = default;
    ~MessageWindow() override = default;

   private:
    DISALLOW_COPY_AND_ASSIGN(MessageWindow);
  };

  TimerController() : message_window_(CreateMessageWindow()) {}

  UINT_PTR ComputeCookie(AbstractTimer* timer) {
    auto const entry = timer->entry_.get();
    DCHECK(entry);
    auto const cookie = reinterpret_cast<UINT_PTR>(entry);
    return cookie;
  }

  static std::unique_ptr<MessageWindow> CreateMessageWindow() {
    std::unique_ptr<MessageWindow> window(new MessageWindow());
    window->CreateWindowEx(0, 0, nullptr, HWND_MESSAGE, common::win::Point(),
                           common::win::Size());
    return std::move(window);
  }

  void SetTimer(AbstractTimer* timer, int next_fire_interval_ms) {
    ::SetTimer(*message_window_, ComputeCookie(timer),
               static_cast<UINT>(next_fire_interval_ms),
               TimerController::TimerProc);
  }

  static void CALLBACK TimerProc(HWND, UINT, UINT_PTR cookie, DWORD) {
    scoped_refptr<TimerEntry> entry = reinterpret_cast<TimerEntry*>(cookie);
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
      TimerController::instance()->SetTimer(timer, entry->repeat_interval_ms);
    else
      TimerController::instance()->StopTimer(timer);
  }

  std::unique_ptr<MessageWindow> message_window_;

  DISALLOW_COPY_AND_ASSIGN(TimerController);
};

AbstractTimer::AbstractTimer() {}

AbstractTimer::~AbstractTimer() {
  if (!entry_)
    return;
  Stop();
}

void AbstractTimer::Start(int next_fire_interval_ms, int repeat_interval_ms) {
  DCHECK_GE(next_fire_interval_ms, 0);
  DCHECK_GE(repeat_interval_ms, 0);
  TimerController::instance()->StartTimer(this, next_fire_interval_ms,
                                          repeat_interval_ms);
}

void AbstractTimer::Stop() {
  TimerController::instance()->StopTimer(this);
}

}  // namespace impl
}  // namespace common
