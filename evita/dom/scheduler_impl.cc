// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <queue>

#include "evita/dom/scheduler_impl.h"

#include "base/synchronization/lock.h"
#include "base/trace_event/trace_event.h"
#include "common/maybe.h"
#include "evita/dom/public/view_event.h"
#include "evita/dom/scheduler_client.h"
#include "evita/dom/script_host.h"

namespace dom {

namespace {

// To make sure we call |SchedulerClient::DidUpdateDom()| after running scripts.
class DomUpdateScope {
 public:
  DomUpdateScope(SchedulerClient* scheduler_client, const base::Time& deadline)
      : deadline_(deadline), scheduler_client_(scheduler_client) {}

  ~DomUpdateScope() { scheduler_client_->DidUpdateDom(); }

 private:
  const base::Time deadline_;
  SchedulerClient* const scheduler_client_;

  DISALLOW_COPY_AND_ASSIGN(DomUpdateScope);
};
}  // namespace

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

//////////////////////////////////////////////////////////////////////
//
// SchedulerImpl
//
SchedulerImpl::SchedulerImpl(SchedulerClient* scheduler_client)
    : idle_task_queue_(new TaskQueue()),
      normal_task_queue_(new TaskQueue()),
      scheduler_client_(scheduler_client),
      view_is_idle_(false) {}

SchedulerImpl::~SchedulerImpl() {}

bool SchedulerImpl::IsViewIdle() const {
  if (!view_is_idle_ || !normal_task_queue_->empty())
    return false;
  return view_idle_deadline_ > base::Time::Now();
}

void SchedulerImpl::RunMicrotasks() {
  TRACE_EVENT0("script", "SchedulerImpl::RunMicrotasks");
  ScriptHost::instance()->RunMicrotasks();
}

// dom::Scheduler
void SchedulerImpl::DidBeginFrame(const base::Time& deadline) {
  TRACE_EVENT1("script", "SchedulerImpl::DidBeginFrame", "period",
               (view_idle_deadline_ - base::Time::Now()).InMilliseconds());
  DomUpdateScope scope(scheduler_client_, deadline);

  for (;;) {
    auto maybe_task = normal_task_queue_->TakeTask();
    if (maybe_task.IsNothing())
      break;
    maybe_task.FromJust().Run();
  }
  RunMicrotasks();
  view_idle_deadline_ = deadline;
  RunMicrotasks();
}

void SchedulerImpl::DidEnterViewIdle(const base::Time& deadline) {
  DCHECK(!view_is_idle_);
  view_idle_deadline_ = deadline;
  view_is_idle_ = true;
}

void SchedulerImpl::DidExitViewIdle() {
  view_is_idle_ = false;
}

void SchedulerImpl::RunIdleTasks() {
  if (idle_task_queue_->empty())
    return;
  TRACE_EVENT1("script", "SchedulerImpl::RunIdleTasks", "period",
               (view_idle_deadline_ - base::Time::Now()).InMilliseconds());
  if (!IsViewIdle())
    return;
  do {
    auto maybe_task = idle_task_queue_->TakeTask();
    if (maybe_task.IsNothing())
      break;
    maybe_task.FromJust().Run();
  } while (IsViewIdle());
  RunMicrotasks();
}

void SchedulerImpl::ScheduleIdleTask(const base::Closure& task) {
  TRACE_EVENT0("script", "SchedulerImpl::ScheduleIdleTask");
  idle_task_queue_->GiveTask(task);
}

void SchedulerImpl::ScheduleTask(const base::Closure& task) {
  TRACE_EVENT0("script", "SchedulerImpl::ScheduleTask");
  normal_task_queue_->GiveTask(task);
}

}  // namespace dom
