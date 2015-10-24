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

Scheduler::Scheduler(base::MessageLoop* message_loop)
    : lock_(new base::Lock()),
      message_loop_(message_loop),
      script_is_running_(false),
      timer_is_running_(false) {}

Scheduler::~Scheduler() {}

void Scheduler::CancelAnimationFrameRequest(
    ui::AnimationFrameHandler* handler) {
  TRACE_EVENT0("scheduler", "Scheduler::CancelAnimationFrameRequest");
  base::AutoLock lock_scope(*lock_);
  pending_handlers_.erase(handler);
  canceled_handlers_.insert(handler);
}

void Scheduler::DidFireTimer() {
  TRACE_EVENT0("scheduler", "Scheduler::DidFireTimer");
  {
    base::AutoLock lock_scope(*lock_);
    timer_is_running_ = false;
  }
  StartTimer();

  auto const now = base::Time::Now();
  HandleAnimationFrame(now);
  if (!script_is_running_)
    StartScript();
  Paint();
}

void Scheduler::DidUpdateDom() {
  TRACE_EVENT0("scheduler", "Scheduler::DidUpdateDom");
  script_is_running_ = false;
}

void Scheduler::HandleAnimationFrame(base::Time time) {
  std::unordered_set<ui::AnimationFrameHandler*> running_handlers;
  std::unordered_set<ui::AnimationFrameHandler*> canceling_handlers;
  {
    base::AutoLock lock_scope(*lock_);
    running_handlers.swap(pending_handlers_);
    canceling_handlers.swap(canceled_handlers_);
  }
  TRACE_EVENT1("scheduler", "Scheduler::HandleAnimationFrame", "tasks",
               running_handlers.size());
  for (auto handler : running_handlers) {
    if (canceling_handlers.find(handler) != canceling_handlers.end())
      continue;
    TRACE_EVENT0("scheduler", "Scheduler::HandleAnimationFrame/1");
    handler->HandleAnimationFrame(time);
  }
}

void Scheduler::Paint() {
  TRACE_EVENT0("scheduler", "Scheduler::Paint");
  ui::Compositor::instance()->CommitIfNeeded();
  last_paint_time_ = base::Time::Now();
}

void Scheduler::RequestAnimationFrame(ui::AnimationFrameHandler* handler) {
  TRACE_EVENT0("scheduler", "Scheduler::RequestAnimationFrame");
  base::AutoLock lock_scope(*lock_);
  pending_handlers_.insert(handler);
}

void Scheduler::Start() {
  TRACE_EVENT0("scheduler", "Scheduler::Start");
  StartTimer();
}

void Scheduler::StartScript() {
  TRACE_EVENT0("scheduler", "Scheduler::StartScript");
  auto const now = base::Time::Now();
  script_start_time_ = now;
  script_is_running_ = true;
  auto const deadline = now + base::TimeDelta::FromMilliseconds(10);
  editor::Application::instance()->view_event_handler()->DidBeginFrame(
      deadline);
}

void Scheduler::StartTimer() {
  TRACE_EVENT0("scheduler", "Scheduler::StartTimer");
  DCHECK(!timer_is_running_);
  timer_is_running_ = true;
  // TODO(eval1749): We should increase |frame_delta| if the application is
  // running in background.
  auto const frame_delta = base::TimeDelta::FromMilliseconds(1000 / 60);
  message_loop_->PostNonNestableDelayedTask(
      FROM_HERE, base::Bind(&Scheduler::DidFireTimer, base::Unretained(this)),
      frame_delta);
}

}  // namespace editor
