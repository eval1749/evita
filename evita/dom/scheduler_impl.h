// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_SCHEDULER_IMPL_H_
#define EVITA_DOM_SCHEDULER_IMPL_H_

#include <memory>
#include <queue>

#include "common/maybe.h"
#include "evita/dom/scheduler.h"

namespace base {
class Lock;
}

namespace dom {

class ViewDelegate;

class SchedulerImpl final : public Scheduler {
 public:
  explicit SchedulerImpl(ViewDelegate* view_delegate);
  ~SchedulerImpl() final;

 private:
  common::Maybe<base::Closure> Take();

  // dom::Scheduler
  void DidBeginFrame(const base::Time& deadline) final;
  void ScheduleTask(const base::Closure& task) final;

  std::unique_ptr<base::Lock> lock_;
  std::queue<base::Closure> task_queue_;
  ViewDelegate* const view_delegate_;

  DISALLOW_COPY_AND_ASSIGN(SchedulerImpl);
};

}  // namespace dom

#endif  // EVITA_DOM_SCHEDULER_IMPL_H_
