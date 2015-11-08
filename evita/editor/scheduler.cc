// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <iterator>
#include <ostream>

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

static base::TimeDelta ComputeDelay(const base::Time& last_frame_time,
                                    const base::Time& now) {
  // TODO(eval1749): We should increase |frame_delta| if the application is
  // running in background.
  auto const kMinFrameDelta = base::TimeDelta::FromMilliseconds(1000 / 60);
  auto const next_frame_time = last_frame_time + kMinFrameDelta;
  auto const delta = next_frame_time - now;
  return std::max(delta, base::TimeDelta::FromMilliseconds(3));
}

#define FOR_EACH_STATE(V) V(Idle) V(Running) V(Sleeping) V(Waiting)

enum class Scheduler::State {
#define V(name) name,
  FOR_EACH_STATE(V)
#undef V
};

const char* StateNameOf(Scheduler::State state) {
  static const char* const names[] = {
#define V(name) #name,
      FOR_EACH_STATE(V)};
#undef V
  auto const it = std::begin(names) + static_cast<size_t>(state);
  return std::end(names) ? *it : "Invalid";
}

std::ostream& operator<<(std::ostream& ostream, Scheduler::State state) {
  return ostream << StateNameOf(state);
}

//////////////////////////////////////////////////////////////////////
//
// Scheduler
//
Scheduler::Scheduler(domapi::ViewEventHandler* script_delegate)
    : lock_(new base::Lock()),
      message_loop_(base::MessageLoop::current()),
      script_delegate_(script_delegate),
      state_(State::Sleeping),
      state_sequence_num_(1) {
  TRACE_EVENT_ASYNC_BEGIN1("view", "ViewState", state_sequence_num_, "state",
                           StateNameOf(state_));
}

Scheduler::~Scheduler() {}

void Scheduler::BeginFrame() {
  DCHECK_EQ(State::Waiting, state_);
  TRACE_EVENT0("scheduler", "Scheduler::BeginFrame");
  last_frame_time_ = base::Time::Now();
  script_delegate_->DidExitViewIdle();
  {
    base::AutoLock lock_scope(*lock_);
    ChangeState(State::Running);
  }
  HandleAnimationFrame();
  CommitFrame();

  base::AutoLock lock_scope(*lock_);
  if (!pending_handlers_.empty()) {
    state_ = State::Sleeping;
    ScheduleNextFrame();
    return;
  }

  // Enter idle state
  auto const now = base::Time::Now();
  ChangeState(State::Idle);
  auto const idle_deadline = now + base::TimeDelta::FromMicroseconds(50);
  script_delegate_->DidEnterViewIdle(idle_deadline);
}

void Scheduler::CancelAnimationFrameRequest(
    ui::AnimationFrameHandler* handler) {
  TRACE_EVENT0("scheduler", "Scheduler::CancelAnimationFrameRequest");
  base::AutoLock lock_scope(*lock_);
  pending_handlers_.erase(handler);
  canceled_handlers_.insert(handler);
}

void Scheduler::ChangeState(State new_state) {
  DCHECK_NE(state_, new_state);
  lock_->AssertAcquired();
  TRACE_EVENT_ASYNC_END0("view", "ViewState", state_sequence_num_);
  state_ = new_state;
  ++state_sequence_num_;
  TRACE_EVENT_ASYNC_BEGIN1("view", "ViewState", state_sequence_num_, "state",
                           StateNameOf(state_));
}

void Scheduler::CommitFrame() {
  DCHECK_EQ(State::Running, state_);
  TRACE_EVENT0("scheduler", "Scheduler::CommitFrame");
  ui::Compositor::instance()->CommitIfNeeded();
  last_paint_time_ = base::Time::Now();
}

void Scheduler::DidUpdateDom() {
  TRACE_EVENT0("scheduler", "Scheduler::DidUpdateDom");
}

void Scheduler::HandleAnimationFrame() {
  DCHECK_EQ(State::Running, state_);
  auto const time = base::Time::Now();
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
  switch (state_) {
    case State::Idle:
      script_delegate_->DidExitViewIdle();
      ChangeState(State::Sleeping);
      ScheduleNextFrame();
      return;
    case State::Running:
    case State::Waiting:
      return;
    case State::Sleeping:
      ScheduleNextFrame();
      return;
  }
  NOTREACHED();
}

void Scheduler::ScheduleNextFrame() {
  lock_->AssertAcquired();
  DCHECK_EQ(State::Sleeping, state_);
  auto const now = base::Time::Now();
  ChangeState(State::Waiting);
  auto const delay = ComputeDelay(last_frame_time_, now);
  script_delegate_->DidEnterViewIdle(now + delay);
  message_loop_->PostNonNestableDelayedTask(
      FROM_HERE, base::Bind(&Scheduler::BeginFrame, base::Unretained(this)),
      delay);
}

void Scheduler::Start() {
  TRACE_EVENT0("scheduler", "Scheduler::Start");
  DCHECK_EQ(State::Waiting, state_);
}

}  // namespace editor
