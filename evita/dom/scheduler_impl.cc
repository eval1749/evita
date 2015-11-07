// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <queue>
#include <unordered_map>

#include "evita/dom/scheduler_impl.h"

#include "base/message_loop/message_loop.h"
#include "base/synchronization/lock.h"
#include "base/trace_event/trace_event.h"
#include "common/maybe.h"
#include "evita/dom/public/view_event.h"
#include "evita/dom/scheduler_client.h"
#include "evita/dom/script_host.h"
#include "evita/dom/timing/idle_deadline_provider.h"
#include "evita/dom/timing/idle_task.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// SchedulerImpl::IdleTaskQueue
//
class SchedulerImpl::IdleTaskQueue final : public IdleDeadlineProvider {
 public:
  IdleTaskQueue();
  ~IdleTaskQueue() = default;

  void CancelTask(int task_id);
  void DidEnterViewIdle(const base::Time& deadline);
  void DidExitViewIdle();
  int GiveTask(const IdleTask& task);
  void RunIdleTasks();
  void StopIdleTasks();

 private:
  void RemoveTask(IdleTask* task);

  // dom::IdleDeadlineProvider
  base::TimeDelta GetTimeRemaining() const final;
  bool IsIdle() const final;

  std::queue<IdleTask*> ready_tasks_;
  volatile bool should_stop_;
  std::unordered_map<int, IdleTask*> task_map_;
  base::Time view_idle_deadline_;
  volatile bool view_is_idle_;
  std::priority_queue<IdleTask*> waiting_tasks_;

  DISALLOW_COPY_AND_ASSIGN(IdleTaskQueue);
};

SchedulerImpl::IdleTaskQueue::IdleTaskQueue()
    : should_stop_(false), view_is_idle_(false) {}

void SchedulerImpl::IdleTaskQueue::CancelTask(int task_id) {
  auto const it = task_map_.find(task_id);
  if (it == task_map_.end())
    return;
  it->second->Cancel();
}

void SchedulerImpl::IdleTaskQueue::DidEnterViewIdle(
    const base::Time& deadline) {
  view_idle_deadline_ = deadline;
  view_is_idle_ = true;
}

void SchedulerImpl::IdleTaskQueue::DidExitViewIdle() {
  view_is_idle_ = false;
}

base::TimeDelta SchedulerImpl::IdleTaskQueue::GetTimeRemaining() const {
  return view_idle_deadline_ - base::Time::Now();
}

int SchedulerImpl::IdleTaskQueue::GiveTask(const IdleTask& task_in) {
  auto const task = new IdleTask(task_in);
  task_map_.insert({task->id(), task});
  if (task->delayed_run_time != base::TimeTicks())
    waiting_tasks_.push(task);
  else
    ready_tasks_.push(task);
  return task->id();
}

bool SchedulerImpl::IdleTaskQueue::IsIdle() const {
  return !should_stop_ && view_is_idle_ &&
         view_idle_deadline_ > base::Time::Now();
}

void SchedulerImpl::IdleTaskQueue::RemoveTask(IdleTask* task) {
  auto const it = task_map_.find(task->id());
  DCHECK(it != task_map_.end());
  task_map_.erase(it);
  delete task;
}

void SchedulerImpl::IdleTaskQueue::RunIdleTasks() {
  should_stop_ = false;

  auto const now = base::TimeTicks::Now();
  while (!waiting_tasks_.empty() &&
         waiting_tasks_.top()->delayed_run_time <= now) {
    ready_tasks_.push(waiting_tasks_.top());
    waiting_tasks_.pop();
  }

  while (!ready_tasks_.empty() && IsIdle()) {
    auto const task = ready_tasks_.front();
    ready_tasks_.pop();
    if (!task->IsCanceled())
      task->Run();
    RemoveTask(task);
  }
}

void SchedulerImpl::IdleTaskQueue::StopIdleTasks() {
  should_stop_ = true;
}

//////////////////////////////////////////////////////////////////////
//
// SchedulerImpl::TaskQueue
//
class SchedulerImpl::TaskQueue {
 public:
  TaskQueue() = default;
  ~TaskQueue() = default;

  bool empty() const { return tasks_.empty(); }
  size_t size() const { return tasks_.size(); }

  void GiveTask(const base::Closure& closure);
  common::Maybe<base::Closure> TakeTask();

 private:
  base::Lock lock_;
  std::queue<base::Closure> tasks_;

  DISALLOW_COPY_AND_ASSIGN(TaskQueue);
};

void SchedulerImpl::TaskQueue::GiveTask(const base::Closure& task) {
  base::AutoLock lock_scope(lock_);
  tasks_.push(task);
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
    : idle_task_queue_(new IdleTaskQueue()),
      normal_task_queue_(new TaskQueue()),
      scheduler_client_(scheduler_client),
      script_message_loop_(nullptr),
      state_(State::Initialized),
      view_message_loop_(base::MessageLoop::current()) {}

SchedulerImpl::~SchedulerImpl() {}

void SchedulerImpl::ProcessTasks() {
  ASSERT_ON_SCRIPT_THREAD();
  state_ = State::Running;
  for (;;) {
    auto maybe_task = normal_task_queue_->TakeTask();
    if (maybe_task.IsNothing()) {
      ScriptHost::instance()->RunMicrotasks();
      if (normal_task_queue_->empty())
        break;
      continue;
    }
    maybe_task.FromJust().Run();
  }
  state_ = State::Sleep;
}

void SchedulerImpl::Start(base::MessageLoop* script_message_loop) {
  ASSERT_ON_VIEW_THREAD();
  DCHECK_NE(view_message_loop_, script_message_loop);
  DCHECK(!script_message_loop_);
  script_message_loop_ = script_message_loop;
}

void SchedulerImpl::StartFrame(const base::Time& deadline) {
  ASSERT_ON_SCRIPT_THREAD();
  TRACE_EVENT0("script", "SchedulerImpl::StartFrame");
  ProcessTasks();
  if (base::Time::Now() < deadline) {
    idle_task_queue_->DidEnterViewIdle(deadline);
    RunIdleTasks();
    idle_task_queue_->DidExitViewIdle();
  }
  scheduler_client_->DidUpdateDom();
}

// dom::Scheduler
void SchedulerImpl::CancelIdleTask(int task_id) {
  idle_task_queue_->CancelTask(task_id);
}

void SchedulerImpl::DidBeginFrame(const base::Time& deadline) {
  ASSERT_ON_VIEW_THREAD();
  script_message_loop_->task_runner()->PostTask(
      FROM_HERE,
      base::Bind(&SchedulerImpl::StartFrame, base::Unretained(this), deadline));
}

void SchedulerImpl::DidEnterViewIdle(const base::Time& deadline) {
  ASSERT_ON_VIEW_THREAD();
  DCHECK(script_message_loop_->task_runner());
  idle_task_queue_->DidEnterViewIdle(deadline);
  script_message_loop_->task_runner()->PostTask(
      FROM_HERE, base::Bind(&Scheduler::RunIdleTasks, base::Unretained(this)));
}

void SchedulerImpl::DidExitViewIdle() {
  idle_task_queue_->DidExitViewIdle();
}

IdleDeadlineProvider* SchedulerImpl::GetIdleDeadlineProvider() {
  return idle_task_queue_.get();
}

void SchedulerImpl::RunIdleTasks() {
  TRACE_EVENT0("script", "SchedulerImpl::RunIdleTasks");
  idle_task_queue_->RunIdleTasks();
}

int SchedulerImpl::ScheduleIdleTask(const IdleTask& task) {
  TRACE_EVENT0("script", "SchedulerImpl::ScheduleIdleTask");
  return idle_task_queue_->GiveTask(task);
}

void SchedulerImpl::ScheduleTask(const base::Closure& task) {
  TRACE_EVENT0("script", "SchedulerImpl::ScheduleTask");
  DCHECK(script_message_loop_->task_runner());
  normal_task_queue_->GiveTask(task);
  idle_task_queue_->StopIdleTasks();
  if (state_ == State::Running)
    return;
  script_message_loop_->task_runner()->PostTask(
      FROM_HERE,
      base::Bind(&SchedulerImpl::ProcessTasks, base::Unretained(this)));
}

}  // namespace dom
