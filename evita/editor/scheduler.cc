// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "evita/editor/scheduler.h"

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/synchronization/lock.h"
#include "base/trace_event/trace_event.h"
#include "evita/dom/public/view_event_handler.h"
#include "evita/editor/application.h"
#include "evita/ui/animation/animation_frame_handler.h"
#include "evita/ui/compositor/compositor.h"

namespace editor {

Scheduler::Scheduler(domapi::ViewEventHandler* script_delegate)
    : is_running_(false),
      lock_(new base::Lock()),
      message_loop_(base::MessageLoop::current()),
      post_task_(false),
      script_delegate_(script_delegate) {}

Scheduler::~Scheduler() {}

void Scheduler::BeginFrame() {
  TRACE_EVENT0("scheduler", "Scheduler::BeginFrame");
  last_frame_time_ = base::Time::Now();
  script_delegate_->DidExitViewIdle();
  {
    base::AutoLock lock_scope(*lock_);
    DCHECK(post_task_);
    is_running_ = true;
    post_task_ = false;
  }
  auto const now = base::Time::Now();
  HandleAnimationFrame(now);
  CommitFrame();

  base::AutoLock lock_scope(*lock_);
  is_running_ = false;
  if (pending_handlers_.empty()) {
    auto const idle_deadline =
        base::Time::Now() + base::TimeDelta::FromMicroseconds(50);
    script_delegate_->DidEnterViewIdle(idle_deadline);
    return;
  }
  ScheduleFrame();
}

void Scheduler::CancelAnimationFrameRequest(
    ui::AnimationFrameHandler* handler) {
  TRACE_EVENT0("scheduler", "Scheduler::CancelAnimationFrameRequest");
  base::AutoLock lock_scope(*lock_);
  pending_handlers_.erase(handler);
  canceled_handlers_.insert(handler);
}

void Scheduler::CommitFrame() {
  TRACE_EVENT0("scheduler", "Scheduler::CommitFrame");
  ui::Compositor::instance()->CommitIfNeeded();
  last_paint_time_ = base::Time::Now();
}

void Scheduler::DidUpdateDom() {
  TRACE_EVENT0("scheduler", "Scheduler::DidUpdateDom");
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
    TRACE_EVENT1("scheduler", "Scheduler::HandleAnimationFrame", "type",
                 handler->GetAnimationFrameType());
    handler->HandleAnimationFrame(time);
  }
}

void Scheduler::RequestAnimationFrame(ui::AnimationFrameHandler* handler) {
  TRACE_EVENT0("scheduler", "Scheduler::RequestAnimationFrame");
  base::AutoLock lock_scope(*lock_);
  pending_handlers_.insert(handler);
  if (is_running_ || post_task_)
    return;
  ScheduleFrame();
}

static base::TimeDelta ComputeDelay(const base::Time& last_frame_time,
                                    const base::Time& now) {
  // TODO(eval1749): We should increase |frame_delta| if the application is
  // running in background.
  auto const kMinFrameDelta = base::TimeDelta::FromMilliseconds(1000 / 60);
  auto const next_frame_time = last_frame_time + kMinFrameDelta;
  auto const delta = next_frame_time - now;
  return std::max(delta, base::TimeDelta::FromMilliseconds(3));
}

void Scheduler::ScheduleFrame() {
  TRACE_EVENT0("scheduler", "Scheduler::ScheduleFrame");
  lock_->AssertAcquired();
  DCHECK(!post_task_);
  post_task_ = true;
  auto const now = base::Time::Now();
  auto const delay = ComputeDelay(last_frame_time_, now);
  script_delegate_->DidEnterViewIdle(now + delay);
  message_loop_->PostNonNestableDelayedTask(
      FROM_HERE, base::Bind(&Scheduler::BeginFrame, base::Unretained(this)),
      delay);
}

void Scheduler::Start() {
  TRACE_EVENT0("scheduler", "Scheduler::Start");
}

}  // namespace editor
