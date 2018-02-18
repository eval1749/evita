// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/scheduler/idle_task_queue.h"

#include <utility>

#include "base/trace_event/trace_event.h"
#include "evita/dom/scheduler/idle_task.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// IdleTaskQueue
//
IdleTaskQueue::IdleTaskQueue() {}
IdleTaskQueue::~IdleTaskQueue() {}

void IdleTaskQueue::CancelTask(int task_id) {
  auto const it = task_map_.find(task_id);
  if (it == task_map_.end())
    return;
  it->second->Cancel();
}

int IdleTaskQueue::GiveTask(IdleTask task_in) {
  auto* const task = new IdleTask(std::move(task_in));
  task_map_.insert({task->id(), task});
  if (task->delayed_run_time() != base::TimeTicks())
    waiting_tasks_.push(task);
  else
    ready_tasks_.push(task);
  return task->id();
}

void IdleTaskQueue::RemoveTask(IdleTask* task) {
  auto const it = task_map_.find(task->id());
  DCHECK(it != task_map_.end());
  task_map_.erase(it);
  delete task;
}

void IdleTaskQueue::RunIdleTasks(const base::TimeTicks& deadline) {
  const auto& now = base::TimeTicks::Now();
  TRACE_EVENT1("script", "IdleTaskQueue::RunIdleTasks", "deadline",
               (deadline - now).InMilliseconds());
  while (!waiting_tasks_.empty() &&
         waiting_tasks_.top()->delayed_run_time() <= now) {
    ready_tasks_.push(waiting_tasks_.top());
    waiting_tasks_.pop();
  }

  // Run runnable tasks before this loop.
  for (auto count = ready_tasks_.size(); count > 0; --count) {
    if (deadline <= base::TimeTicks::Now())
      break;
    if (should_stop_.load())
      break;
    auto* const task = ready_tasks_.front();
    ready_tasks_.pop();
    if (!task->IsCanceled())
      task->Run(deadline);
    RemoveTask(task);
  }
  should_stop_.store(false);
}

void IdleTaskQueue::StopIdleTasks() {
  should_stop_.store(true);
}

}  // namespace dom
