// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <queue>
#include <unordered_map>
#include <vector>

#include "evita/dom/scheduler/scheduler_impl.h"

#include "base/message_loop/message_loop.h"
#include "base/synchronization/lock.h"
#include "base/trace_event/trace_event.h"
#include "common/maybe.h"
#include "evita/dom/scheduler/animation_frame_callback.h"
#include "evita/dom/scheduler/idle_task_queue.h"
#include "evita/dom/scheduler/scheduler_client.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// SchedulerImpl::AnimationFrameCallbackQueue
//
class SchedulerImpl::AnimationFrameCallbackQueue final {
 public:
  AnimationFrameCallbackQueue() = default;
  ~AnimationFrameCallbackQueue() = default;

  void Cancel(int callback_id);
  void DidBeginAnimationFrame(const base::TimeTicks& time);
  int Give(std::unique_ptr<AnimationFrameCallback> callback);

 private:
  int last_callback_id_ = 0;
  std::unordered_map<int, std::unique_ptr<AnimationFrameCallback>>
      callback_map_;

  DISALLOW_COPY_AND_ASSIGN(AnimationFrameCallbackQueue);
};

void SchedulerImpl::AnimationFrameCallbackQueue::Cancel(int callback_id) {
  const auto& it = callback_map_.find(callback_id);
  if (it == callback_map_.end())
    return;
  callback_map_.erase(it);
}

void SchedulerImpl::AnimationFrameCallbackQueue::DidBeginAnimationFrame(
    const base::TimeTicks& time) {
  std::vector<std::unique_ptr<AnimationFrameCallback>> callbacks;
  callbacks.reserve(callback_map_.size());
  for (auto& pair : callback_map_)
    callbacks.emplace_back(std::move(pair.second));
  callback_map_.clear();
  for (const auto& callback : callbacks)
    callback->Run(time);
}

int SchedulerImpl::AnimationFrameCallbackQueue::Give(
    std::unique_ptr<AnimationFrameCallback> callback) {
  ++last_callback_id_;
  callback_map_.emplace(last_callback_id_, std::move(callback));
  return last_callback_id_;
}

//////////////////////////////////////////////////////////////////////
//
// SchedulerImpl::TaskQueue
//
class SchedulerImpl::TaskQueue {
 public:
  TaskQueue() = default;
  ~TaskQueue() = default;

  // Returns true if there is only one task in queue.
  bool GiveTask(const base::Closure& closure);
  bool IsEmpty() const;
  common::Maybe<base::Closure> TakeTask();

 private:
  mutable base::Lock lock_;
  std::queue<base::Closure> tasks_;

  DISALLOW_COPY_AND_ASSIGN(TaskQueue);
};

bool SchedulerImpl::TaskQueue::GiveTask(const base::Closure& task) {
  base::AutoLock lock_scope(lock_);
  tasks_.push(task);
  return tasks_.size() == 1;
}

bool SchedulerImpl::TaskQueue::IsEmpty() const {
  base::AutoLock lock_scope(lock_);
  return tasks_.empty();
}

common::Maybe<base::Closure> SchedulerImpl::TaskQueue::TakeTask() {
  base::AutoLock lock_scope(lock_);
  if (tasks_.empty())
    return common::Nothing<base::Closure>();
  auto task = common::Just(tasks_.front());
  tasks_.pop();
  return task;
}

enum class SchedulerImpl::State {
  Initialized,
  Running,
  Sleep,
};

//////////////////////////////////////////////////////////////////////
//
// SchedulerImpl
//
#define ASSERT_ON_VIEW_THREAD() \
  DCHECK_EQ(view_message_loop_, base::MessageLoop::current())

#define ASSERT_ON_SCRIPT_THREAD() \
  DCHECK_EQ(script_message_loop_, base::MessageLoop::current())

SchedulerImpl::SchedulerImpl(SchedulerClient* scheduler_client)
    : animation_frame_callback_queue_(new AnimationFrameCallbackQueue()),
      idle_task_queue_(new IdleTaskQueue()),
      normal_task_queue_(new TaskQueue()),
      scheduler_client_(scheduler_client),
      script_message_loop_(nullptr),
      state_(State::Initialized),
      view_message_loop_(base::MessageLoop::current()) {}

SchedulerImpl::~SchedulerImpl() {}

void SchedulerImpl::BeginFrame(const base::TimeTicks& deadline) {
  ASSERT_ON_SCRIPT_THREAD();
  TRACE_EVENT0("script", "SchedulerImpl::BeginFrame");
  animation_frame_callback_queue_->DidBeginAnimationFrame(
      base::TimeTicks::Now());
  if (base::TimeTicks::Now() < deadline)
    idle_task_queue_->RunIdleTasks(deadline);
  scheduler_client_->EnterIdle(deadline);
}

void SchedulerImpl::ProcessTasks() {
  TRACE_EVENT0("script", "SchedulerImpl::ProcessTasks");
  ASSERT_ON_SCRIPT_THREAD();
  state_.store(State::Running);
  for (;;) {
    auto maybe_task = normal_task_queue_->TakeTask();
    if (maybe_task.IsNothing()) {
      scheduler_client_->RunMicrotasks();
      if (normal_task_queue_->IsEmpty())
        break;
      continue;
    }
    maybe_task.FromJust().Run();
  }
  state_.store(State::Sleep);
}

void SchedulerImpl::Start(base::MessageLoop* script_message_loop) {
  ASSERT_ON_VIEW_THREAD();
  DCHECK_NE(view_message_loop_, script_message_loop);
  DCHECK(!script_message_loop_);
  script_message_loop_ = script_message_loop;
}

// base::TickClock
base::TimeTicks SchedulerImpl::NowTicks() {
  return base::TimeTicks::Now();
}

// dom::Scheduler
void SchedulerImpl::CancelAnimationFrame(int callback_id) {
  animation_frame_callback_queue_->Cancel(callback_id);
  scheduler_client_->DidCancelAnimationFrame();
}

void SchedulerImpl::CancelIdleTask(int task_id) {
  idle_task_queue_->CancelTask(task_id);
}

void SchedulerImpl::DidBeginFrame(const base::TimeTicks& deadline) {
  ASSERT_ON_VIEW_THREAD();
  script_message_loop_->task_runner()->PostTask(
      FROM_HERE,
      base::Bind(&SchedulerImpl::BeginFrame, base::Unretained(this), deadline));
}

int SchedulerImpl::RequestAnimationFrame(
    std::unique_ptr<AnimationFrameCallback> callback) {
  TRACE_EVENT0("script", "SchedulerImpl::RequestAnimationFrame");
  const auto callback_id =
      animation_frame_callback_queue_->Give(std::move(callback));
  scheduler_client_->DidRequestAnimationFrame();
  return callback_id;
}

int SchedulerImpl::ScheduleIdleTask(const IdleTask& task) {
  TRACE_EVENT0("script", "SchedulerImpl::ScheduleIdleTask");
  return idle_task_queue_->GiveTask(task);
}

void SchedulerImpl::ScheduleTask(const base::Closure& task) {
  TRACE_EVENT0("script", "SchedulerImpl::ScheduleTask");
  DCHECK(script_message_loop_->task_runner());
  if (normal_task_queue_->GiveTask(task)) {
    // Since file I/O is done in less than 1ms, we should run a task to
    // schedule next file I/O rather than waiting animation frame.
    script_message_loop_->task_runner()->PostTask(
        FROM_HERE,
        base::Bind(&SchedulerImpl::ProcessTasks, base::Unretained(this)));
  }
  idle_task_queue_->StopIdleTasks();
}

}  // namespace dom
