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
#include "evita/ui/focus_controller.h"

namespace editor {

// Note: To update view after blur, we don't use longer period for
// non-foreground window.
static base::TimeDelta ComputeFrameDelay() {
  return base::TimeDelta::FromMilliseconds(1000 / 60);
}

static base::TimeDelta ComputeFrameDelay(const base::Time& last_frame_time,
                                         const base::Time& now) {
  auto const next_frame_time = last_frame_time + ComputeFrameDelay();
  auto const delta = next_frame_time - now;
  return std::max(delta, base::TimeDelta::FromMilliseconds(3));
}

static base::TimeDelta ComputeIdleDelay() {
  return ui::FocusController::instance()->IsForeground()
             ? base::TimeDelta::FromMilliseconds(50)
             : base::TimeDelta::FromMilliseconds(5000);
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
    : frame_id_(0),
      idle_sequence_num_(0),
      lock_(new base::Lock()),
      message_loop_(base::MessageLoop::current()),
      script_delegate_(script_delegate),
      state_(State::Sleeping),
      state_sequence_num_(1) {
  TRACE_EVENT_ASYNC_BEGIN1("view", "ViewState", state_sequence_num_, "state",
                           StateNameOf(state_));
  ui::AnimationScheduler::GetInstance()->SetScheduler(this);
}

Scheduler::~Scheduler() {}

void Scheduler::BeginFrame() {
  DCHECK_EQ(State::Waiting, state_);
  const auto& now = base::Time::Now();
  TRACE_EVENT_WITH_FLOW1("scheduler", "Scheduler::BeginFrame", frame_id_,
                         TRACE_EVENT_FLAG_FLOW_IN, "delay",
                         (now - last_frame_time_).InMillisecondsF());
  ++frame_id_;
  last_frame_time_ = now;
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
  ChangeState(State::Idle);
  idle_sequence_num_ = state_sequence_num_;
  EnterIdle();
}

void Scheduler::CancelAnimationFrameRequest(
    ui::AnimationFrameHandler* handler) {
  TRACE_EVENT_WITH_FLOW1("scheduler", "Scheduler::CancelAnimationFrame",
                         handler, TRACE_EVENT_FLAG_FLOW_IN, "type",
                         handler->GetAnimationFrameType());
  base::AutoLock lock_scope(*lock_);
  pending_handlers_.erase(handler);
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

void Scheduler::EnterIdle() {
  lock_->AssertAcquired();
  DCHECK_EQ(State::Idle, state_);
  auto const now = base::Time::Now();
  auto const idle_deadline = now + base::TimeDelta::FromMilliseconds(50);
  script_delegate_->DidEnterViewIdle(idle_deadline);
  message_loop_->task_runner()->PostNonNestableDelayedTask(
      FROM_HERE, base::Bind(&Scheduler::ExitIdle, base::Unretained(this)),
      ComputeIdleDelay());
}

void Scheduler::ExitIdle() {
  base::AutoLock lock_scope(*lock_);
  if (state_ != State::Idle || idle_sequence_num_ != state_sequence_num_)
    return;
  DCHECK_EQ(State::Idle, state_);
  EnterIdle();
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
    TRACE_EVENT_WITH_FLOW1("scheduler", "Scheduler::HandleAnimationFrame",
                           handler, TRACE_EVENT_FLAG_FLOW_IN, "type",
                           handler->GetAnimationFrameType());
    handler->HandleAnimationFrame(time);
  }
}

void Scheduler::RequestAnimationFrame(ui::AnimationFrameHandler* handler) {
  base::AutoLock lock_scope(*lock_);
  const auto& result = pending_handlers_.insert(handler);
  if (!result.second)
    return;
  TRACE_EVENT_WITH_FLOW1("scheduler", "Scheduler::RequestAnimationFrame",
                         handler, TRACE_EVENT_FLAG_FLOW_OUT, "type",
                         handler->GetAnimationFrameType());
  switch (state_) {
    case State::Idle:
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
  auto const delay = ComputeFrameDelay(last_frame_time_, now);
  script_delegate_->DidEnterViewIdle(now + delay);
  TRACE_EVENT_WITH_FLOW1("scheduler", "Scheduler::ScheduleNextFrame", frame_id_,
                         TRACE_EVENT_FLAG_FLOW_OUT, "delay",
                         delay.InMillisecondsF());
  message_loop_->PostNonNestableDelayedTask(
      FROM_HERE, base::Bind(&Scheduler::BeginFrame, base::Unretained(this)),
      delay);
}

void Scheduler::Start() {
  TRACE_EVENT0("scheduler", "Scheduler::Start");
  DCHECK_EQ(State::Waiting, state_);
}

}  // namespace editor
