// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "evita/dom/timing/mock_scheduler.h"

#include "evita/dom/timing/animation_frame_callback.h"
#include "evita/dom/timing/idle_deadline_provider.h"
#include "evita/dom/timing/idle_task.h"

namespace dom {

MockScheduler::MockScheduler() : did_timeout_(false) {}
MockScheduler::~MockScheduler() {}

base::TimeTicks MockScheduler::Now() const {
  return base::TimeTicks::Now() + time_shift_;
}

void MockScheduler::RunPendingTasks() {
  {
    std::vector<std::unique_ptr<AnimationFrameCallback>> callbacks;
    callbacks.reserve(animation_frame_callback_map_.size());
    for (auto& pair : animation_frame_callback_map_)
      callbacks.emplace_back(std::move(pair.second));
    animation_frame_callback_map_.clear();
    const auto& frame_time = base::Time::Now();
    for (const auto& callback : callbacks)
      callback->Run(frame_time);
  }

  while (!normal_tasks_.empty()) {
    normal_tasks_.front().Run();
    normal_tasks_.pop();
  }

  auto const now = Now();
  while (!waiting_idle_tasks_.empty() &&
         waiting_idle_tasks_.front()->delayed_run_time <= now) {
    ready_idle_tasks_.push(waiting_idle_tasks_.front());
    waiting_idle_tasks_.pop();
  }

  while (!ready_idle_tasks_.empty()) {
    auto const idle_task = ready_idle_tasks_.front();
    ready_idle_tasks_.pop();
    if (!idle_task->IsCanceled())
      idle_task->Run();
    idle_task_map_.erase(idle_task_map_.find(idle_task->id()));
    delete idle_task;
  }
}

void MockScheduler::SetIdleDeadline(bool did_timeout,
                                    base::TimeDelta time_remaining) {
  did_timeout_ = did_timeout;
  time_remaining_ = time_remaining;
}

// dom::Scheduler
void MockScheduler::CancelAnimationFrame(int callback_id) {
  const auto& it = animation_frame_callback_map_.find(callback_id);
  if (it == animation_frame_callback_map_.end())
    return;
  animation_frame_callback_map_.erase(it);
}

void MockScheduler::CancelIdleTask(int task_id) {
  auto it = idle_task_map_.find(task_id);
  if (it == idle_task_map_.end())
    return;
  it->second->Cancel();
}

int MockScheduler::RequestAnimationFrame(
    std::unique_ptr<AnimationFrameCallback> callback) {
  ++last_animation_frame_callback_id_;
  animation_frame_callback_map_.emplace(last_animation_frame_callback_id_,
                                        std::move(callback));
  return last_animation_frame_callback_id_;
}

int MockScheduler::ScheduleIdleTask(const IdleTask& task) {
  auto const idle_task = new IdleTask(task);
  if (idle_task->delayed_run_time == base::TimeTicks())
    ready_idle_tasks_.push(idle_task);
  else
    waiting_idle_tasks_.push(idle_task);
  idle_task_map_.insert({idle_task->id(), idle_task});
  return idle_task->id();
}

void MockScheduler::ScheduleTask(const base::Closure& task) {
  normal_tasks_.push(task);
}

}  // namespace dom
