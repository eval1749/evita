// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_SCHEDULER_SCHEDULER_IMPL_H_
#define EVITA_DOM_SCHEDULER_SCHEDULER_IMPL_H_

#include <atomic>
#include <memory>
#include <queue>

#include "base/time/time.h"
#include "evita/dom/scheduler/scheduler.h"

namespace base {
class MessageLoop;
}

namespace dom {

class IdleTask;
class IdleTaskQueue;
class SchedulerClient;

class SchedulerImpl final : public Scheduler {
 public:
  explicit SchedulerImpl(SchedulerClient* scheduler_client);
  ~SchedulerImpl() final;

  void Start(base::MessageLoop* script_message_loop);

 private:
  class AnimationFrameCallbackQueue;
  enum class State;
  class TaskQueue;

  void BeginFrame(const base::TimeTicks& deadline);
  void ProcessTasks();

  // base::TickClock
  base::TimeTicks NowTicks() final;

  // dom::Scheduler
  void CancelAnimationFrame(int request_id) final;
  void CancelIdleTask(int task_id) final;
  void DidBeginFrame(const base::TimeTicks& deadline) final;
  int RequestAnimationFrame(
      std::unique_ptr<AnimationFrameCallback> request) final;
  int ScheduleIdleTask(IdleTask task) final;
  void ScheduleTask(base::OnceClosure task) final;

  std::unique_ptr<AnimationFrameCallbackQueue> animation_frame_callback_queue_;
  std::unique_ptr<IdleTaskQueue> idle_task_queue_;
  std::unique_ptr<TaskQueue> normal_task_queue_;
  SchedulerClient* const scheduler_client_;
  base::MessageLoop* script_message_loop_;
  std::atomic<State> state_;
  base::MessageLoop* const view_message_loop_;

  DISALLOW_COPY_AND_ASSIGN(SchedulerImpl);
};

}  // namespace dom

#endif  // EVITA_DOM_SCHEDULER_SCHEDULER_IMPL_H_
