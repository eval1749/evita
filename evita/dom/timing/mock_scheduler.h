// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TIMING_MOCK_SCHEDULER_H_
#define EVITA_DOM_TIMING_MOCK_SCHEDULER_H_

#include <queue>
#include <unordered_map>

#include "base/macros.h"
#include "base/time/time.h"
#include "evita/dom/scheduler.h"
#include "evita/dom/timing/idle_deadline_provider.h"

namespace dom {

class IdleTask;

//////////////////////////////////////////////////////////////////////
//
// MockScheduler
//
class MockScheduler final : public IdleDeadlineProvider, public Scheduler {
 public:
  MockScheduler();
  ~MockScheduler() final;

  void RunPendingTasks();
  void SetIdleDeadline(bool did_timeout, base::TimeDelta time_remaining);
  void SetTimeShift(base::TimeDelta time_shift) { time_shift_ = time_shift; }

 private:
  base::TimeTicks Now() const;

  // dom::IdleDeadlineProvider
  base::TimeDelta GetTimeRemaining() const final { return time_remaining_; }
  bool IsIdle() const final { return !did_timeout_; }

  // dom::Scheduler
  void CancelIdleTask(int task_id) final;
  void DidBeginFrame(const base::Time& deadline) final { NOTREACHED(); }
  void DidEnterViewIdle(const base::Time& deadline) final { NOTREACHED(); }
  void DidExitViewIdle() final { NOTREACHED(); }
  IdleDeadlineProvider* GetIdleDeadlineProvider() final { return this; }
  void RunIdleTasks() final { NOTREACHED(); }
  int ScheduleIdleTask(const IdleTask& task) final;
  void ScheduleTask(const base::Closure& task) final;

  bool did_timeout_;
  std::queue<IdleTask*> ready_idle_tasks_;
  std::queue<base::Closure> normal_tasks_;
  std::unordered_map<int, IdleTask*> idle_task_map_;
  base::TimeDelta time_remaining_;
  base::TimeDelta time_shift_;
  std::queue<IdleTask*> waiting_idle_tasks_;

  DISALLOW_COPY_AND_ASSIGN(MockScheduler);
};

}  // namespace dom

#endif  // EVITA_DOM_TIMING_MOCK_SCHEDULER_H_
