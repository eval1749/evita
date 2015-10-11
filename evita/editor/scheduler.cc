// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/editor/scheduler.h"

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/synchronization/lock.h"
#include "base/time/time.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/ui/animation/animation_frame_handler.h"
#include "evita/ui/compositor/compositor.h"

namespace editor {

enum class Scheduler::State {
  Running,
  Sleeping,
  Waiting,
};

Scheduler::Scheduler(base::MessageLoop* message_loop)
    : lock_(new base::Lock()),
      message_loop_(message_loop),
      state_(State::Sleeping) {}

Scheduler::~Scheduler() {}

void Scheduler::CancelAnimationFrameRequest(
    ui::AnimationFrameHandler* handler) {
  base::AutoLock lock_scope(*lock_);
  pending_handlers_.erase(handler);
  canceled_handlers_.insert(handler);
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

void Scheduler::Run() {
  base::Time deadline =
      base::Time::Now() + base::TimeDelta::FromMilliseconds(10);
  editor::Application::instance()->view_event_handler()->DidBeginFrame(
      deadline);
  {
    base::AutoLock lock_scope(*lock_);
    state_ = State::Running;
  }
  auto const now = base::Time::Now();
  HandleAnimationFrame(now);
  ui::Compositor::instance()->CommitIfNeeded();
  {
    base::AutoLock lock_scope(*lock_);
    state_ = State::Sleeping;
    if (pending_handlers_.empty())
      return;
    // TODO(eval1749) We should run next frame once composition finished.
    Wait();
  }
}

void Scheduler::RequestAnimationFrame(ui::AnimationFrameHandler* handler) {
  base::AutoLock lock_scope(*lock_);
  pending_handlers_.insert(handler);
  if (state_ != State::Sleeping)
    return;
  Wait();
}

void Scheduler::Wait() {
  DCHECK_EQ(static_cast<int>(state_), static_cast<int>(State::Sleeping));
  state_ = State::Waiting;
  message_loop_->PostNonNestableDelayedTask(
      FROM_HERE, base::Bind(&Scheduler::Run, base::Unretained(this)),
      base::TimeDelta::FromMilliseconds(1000 / 60));
}

}  // namespace editor
