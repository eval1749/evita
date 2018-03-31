// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TIMING_MOCK_SCHEDULER_H_
#define EVITA_DOM_TIMING_MOCK_SCHEDULER_H_

#include <memory>
#include <queue>
#include <unordered_map>

#include "base/macros.h"
#include "base/time/time.h"
#include "evita/dom/scheduler/scheduler.h"

namespace dom {

class IdleTask;

//////////////////////////////////////////////////////////////////////
//
// MockScheduler
//
class MockScheduler final : public Scheduler {
 public:
  MockScheduler();
  ~MockScheduler() final;

  void RunPendingTasks();
  void SetIdleTimeRemaining(int milliseconds);
  void SetNowTicks(int milliseconds);

 private:
  // base::TickClock
  base::TimeTicks NowTicks() const final;

  // dom::Scheduler
  void CancelAnimationFrame(int callback_id) final;
  void CancelIdleTask(int task_id) final;
  void DidBeginFrame(const base::TimeTicks& deadline) final;
  int RequestAnimationFrame(
      std::unique_ptr<AnimationFrameCallback> callback) final;
  int ScheduleIdleTask(IdleTask task) final;
  void ScheduleTask(base::OnceClosure task) final;

  std::unordered_map<int, std::unique_ptr<AnimationFrameCallback>>
      animation_frame_callback_map_;
  base::TimeDelta idle_time_remaining_;
  int last_animation_frame_callback_id_ = 0;
  std::queue<IdleTask*> ready_idle_tasks_;
  std::queue<base::OnceClosure> normal_tasks_;
  std::unordered_map<int, IdleTask*> idle_task_map_;
  base::TimeTicks now_ticks_;
  std::queue<IdleTask*> waiting_idle_tasks_;

  DISALLOW_COPY_AND_ASSIGN(MockScheduler);
};

}  // namespace dom

#endif  // EVITA_DOM_TIMING_MOCK_SCHEDULER_H_
