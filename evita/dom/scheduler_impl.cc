// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <queue>

#include "evita/dom/scheduler_impl.h"

#include "base/synchronization/lock.h"
#include "common/maybe.h"
#include "evita/dom/public/view_event.h"
#include "evita/dom/script_host.h"
#include "evita/dom/view_delegate.h"

namespace dom {

namespace {

// To make sure we call |ViewDelegate::DidUpdateDom()| after running scripts.
class DomUpdateScope {
 public:
  DomUpdateScope(dom::ViewDelegate* view_delegate, const base::Time& deadline)
      : deadline_(deadline), state_("normal"), view_delegate_(view_delegate) {}

  ~DomUpdateScope() {
    view_delegate_->DidUpdateDom();
    auto const delta = base::Time::Now() - deadline_;
    if (delta >= base::TimeDelta::FromMilliseconds(0))
      return;
    DVLOG(0) << "Exceed deadline " << -delta.InMilliseconds() << "ms at "
             << state_;
  }

  void SetState(const char* state) { state_ = state; }

 private:
  const base::Time deadline_;
  const char* state_;
  dom::ViewDelegate* const view_delegate_;

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

  void GiveTask(const base::Closure& closure);

  // Returns true if no more tasks.
  bool RunTasksUntil(const base::Time& deadline);

 private:
  common::Maybe<base::Closure> TakeTask();

  base::Lock lock_;
  std::queue<base::Closure> tasks_;

  DISALLOW_COPY_AND_ASSIGN(TaskQueue);
};

void SchedulerImpl::TaskQueue::GiveTask(const base::Closure& task) {
  base::AutoLock lock_scope(lock_);
  tasks_.push(task);
}

bool SchedulerImpl::TaskQueue::RunTasksUntil(const base::Time& deadline) {
  while (base::Time::Now() < deadline) {
    auto maybe_task = TakeTask();
    if (maybe_task.IsNothing())
      return true;
    maybe_task.FromJust().Run();
  }
  return false;
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
SchedulerImpl::SchedulerImpl(ViewDelegate* view_delegate)
    : idle_task_queue_(new TaskQueue()),
      normal_task_queue_(new TaskQueue()),
      view_delegate_(view_delegate) {}

SchedulerImpl::~SchedulerImpl() {}

void SchedulerImpl::DidBeginFrame(const base::Time& deadline) {
  DomUpdateScope scope(view_delegate_, deadline);

  if (!normal_task_queue_->RunTasksUntil(deadline))
    return;
  scope.SetState("normal microtasks");
  if (!RunMicrotasksIfPossible(deadline))
    return;

  scope.SetState("idle");
  if (!idle_task_queue_->RunTasksUntil(deadline))
    return;
  scope.SetState("idle microtasks");
  RunMicrotasksIfPossible(deadline);
}

bool SchedulerImpl::RunMicrotasksIfPossible(const base::Time& deadline) {
  if (base::Time::Now() >= deadline)
    return false;
  ScriptHost::instance()->RunMicrotasks();
  return true;
}

void SchedulerImpl::ScheduleIdleTask(const base::Closure& task) {
  idle_task_queue_->GiveTask(task);
}

void SchedulerImpl::ScheduleTask(const base::Closure& task) {
  normal_task_queue_->GiveTask(task);
}

}  // namespace dom
