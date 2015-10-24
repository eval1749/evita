// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_SCHEDULER_IMPL_H_
#define EVITA_DOM_SCHEDULER_IMPL_H_

#include <memory>

#include "base/time/time.h"
#include "evita/dom/scheduler.h"

namespace dom {

class SchedulerClient;

class SchedulerImpl final : public Scheduler {
 public:
  explicit SchedulerImpl(SchedulerClient* scheduler_client);
  ~SchedulerImpl() final;

  void RunIdleTasks();

 private:
  class TaskQueue;

  bool IsViewIdle() const;
  void RunMicrotasks();

  // dom::Scheduler
  void DidBeginFrame(const base::Time& deadline) final;
  void DidEnterViewIdle(const base::Time& deadline) final;
  void DidExitViewIdle() final;
  void ScheduleTask(const base::Closure& task) final;
  void ScheduleIdleTask(const base::Closure& task) final;

  std::unique_ptr<TaskQueue> idle_task_queue_;
  std::unique_ptr<TaskQueue> normal_task_queue_;
  SchedulerClient* const scheduler_client_;
  base::Time view_idle_deadline_;
  bool view_is_idle_;

  DISALLOW_COPY_AND_ASSIGN(SchedulerImpl);
};

}  // namespace dom

#endif  // EVITA_DOM_SCHEDULER_IMPL_H_
