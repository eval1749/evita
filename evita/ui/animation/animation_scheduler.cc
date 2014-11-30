// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ui/animation/animation_scheduler.h"

#include "base/bind.h"
#pragma warning(push)
#pragma warning(disable: 4100 4625 4626)
#include "base/message_loop/message_loop.h"
#pragma warning(pop)
#include "base/synchronization/lock.h"
#include "base/time/time.h"
#include "evita/ui/animation/animation_frame_handler.h"
#include "evita/ui/compositor/compositor.h"

namespace ui {

enum class AnimationScheduler::State {
  Running,
  Sleep,
  Waiting,
};

AnimationScheduler::AnimationScheduler(base::MessageLoop* message_loop)
    : lock_(new base::Lock()), message_loop_(message_loop),
      state_(State::Sleep)  {
}

AnimationScheduler::~AnimationScheduler() {
}

void AnimationScheduler::CancelAnimationFrameRequest(
    AnimationFrameHandler* handler) {
  base::AutoLock lock_scope(*lock_);
  pending_handlers_.erase(handler);
  canceled_handlers_.insert(handler);
}

void AnimationScheduler::HandleAnimationFrame(base::Time time) {
  std::unordered_set<AnimationFrameHandler*> running_handlers;
  std::unordered_set<AnimationFrameHandler*> canceling_handlers;
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

void AnimationScheduler::Run() {
  {
    base::AutoLock lock_scope(*lock_);
    state_ = State::Running;
  }
  auto const now = base::Time::Now();
  HandleAnimationFrame(now);
  ui::Compositor::instance()->CommitIfNeeded();
  {
    base::AutoLock lock_scope(*lock_);
    state_ = State::Sleep;
    if (pending_handlers_.empty())
      return;
    // TODO(eval1749) We should run next frame once composition finished.
    Wait();
  }
}

void AnimationScheduler::RequestAnimationFrame(
    AnimationFrameHandler* handler) {
  base::AutoLock lock_scope(*lock_);
  pending_handlers_.insert(handler);
  if (state_ != State::Sleep)
    return;
  Wait();
}

void AnimationScheduler::Wait() {
  DCHECK_EQ(static_cast<int>(state_), static_cast<int>(State::Sleep));
  state_ = State::Waiting;
  message_loop_->PostNonNestableDelayedTask(FROM_HERE,
      base::Bind(&AnimationScheduler::Run, base::Unretained(this)),
      base::TimeDelta::FromMilliseconds(1000 / 60));
}

}  // namespace ui
