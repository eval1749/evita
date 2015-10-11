// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/editor/scheduler.h"

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/synchronization/lock.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/ui/animation/animation_frame_handler.h"
#include "evita/ui/compositor/compositor.h"

namespace editor {

Scheduler::Scheduler(base::MessageLoop* message_loop)
    : lock_(new base::Lock()),
      message_loop_(message_loop),
      script_is_running_(true),
      timer_is_running_(false) {}

Scheduler::~Scheduler() {}

void Scheduler::CancelAnimationFrameRequest(
    ui::AnimationFrameHandler* handler) {
  base::AutoLock lock_scope(*lock_);
  pending_handlers_.erase(handler);
  canceled_handlers_.insert(handler);
}

void Scheduler::DidFireTimer() {
  {
    base::AutoLock lock_scope(*lock_);
    timer_is_running_ = false;
  }

  auto const now = base::Time::Now();

  if (script_is_running_) {
    // TODO(eval1749): Run non-DOM animations
    DVLOG(0) << "Scheduler::DidFirTimer: script_is_running_"
             << " paint=" << (now - last_paint_time_).InMillisecondsF() << "ms"
             << " script=" << (now - script_start_time_).InMillisecondsF()
             << "ms";
    RunAnimation();
    return;
  }

  script_start_time_ = now;
  script_is_running_ = true;
  auto const deadline =
      base::Time::Now() + base::TimeDelta::FromMilliseconds(10);
  editor::Application::instance()->view_event_handler()->DidBeginFrame(
      deadline);
}

void Scheduler::DidUpdateDom() {
  script_is_running_ = false;
  RunAnimation();
}

void Scheduler::RunAnimation() {
  HandleAnimationFrame(base::Time::Now());
  ui::Compositor::instance()->CommitIfNeeded();
  last_paint_time_ = base::Time::Now();
  StartTimerIfNeeded();
}

void Scheduler::HandleAnimationFrame(base::Time time) {
  std::unordered_set<ui::AnimationFrameHandler*> running_handlers;
  std::unordered_set<ui::AnimationFrameHandler*> canceling_handlers;
  {
    base::AutoLock lock_scope(*lock_);
    running_handlers.swap(pending_handlers_);
    canceling_handlers.swap(canceled_handlers_);
  }
  for (auto handler : running_handlers) {
    if (canceling_handlers.find(handler) != canceling_handlers.end())
      continue;
    handler->HandleAnimationFrame(time);
  }
}

void Scheduler::RequestAnimationFrame(ui::AnimationFrameHandler* handler) {
  {
    base::AutoLock lock_scope(*lock_);
    pending_handlers_.insert(handler);
  }
  StartTimerIfNeeded();
}

void Scheduler::StartTimerIfNeeded() {
  base::AutoLock lock_scope(*lock_);
  if (timer_is_running_)
    return;
  timer_is_running_ = true;
  auto const kFrameDelta = base::TimeDelta::FromMilliseconds(1000 / 60);
  auto const next_paint_time = last_paint_time_ + kFrameDelta;
  auto const remaining = next_paint_time - base::Time::Now();
  message_loop_->PostNonNestableDelayedTask(
      FROM_HERE, base::Bind(&Scheduler::DidFireTimer, base::Unretained(this)),
      std::max(base::TimeDelta::FromMilliseconds(0), remaining));
}

}  // namespace editor
