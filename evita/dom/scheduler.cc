// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/scheduler.h"

#include "base/synchronization/lock.h"
#include "evita/dom/public/view_event.h"
#include "evita/dom/script_host.h"
#include "evita/dom/view_delegate.h"

namespace dom {

Scheduler::Scheduler(ViewDelegate* view_delegate)
    : lock_(new base::Lock()), view_delegate_(view_delegate) {}

Scheduler::~Scheduler() {}

void Scheduler::DidBeginFrame(const base::Time& deadline) {
  for (;;) {
    auto maybe_task = Take();
    if (maybe_task.IsNothing()) {
      ScriptHost::instance()->RunMicrotasks();
      break;
    }
    maybe_task.FromJust().Run();
    if (base::Time::Now() > deadline) {
      base::AutoLock lock_scope(*lock_);
      if (!task_queue_.empty()) {
        DVLOG(0) << "Reached at deadline, " << task_queue_.size()
                 << " tasks left";
      }
      break;
    }
  }
  view_delegate_->DidUpdateDom();
}

void Scheduler::ScheduleTask(const base::Closure& task) {
  base::AutoLock lock_scope(*lock_);
  task_queue_.push(task);
}

common::Maybe<base::Closure> Scheduler::Take() {
  base::AutoLock lock_scope(*lock_);
  if (task_queue_.empty())
    return common::Nothing<base::Closure>();
  auto task = common::Just(task_queue_.front());
  task_queue_.pop();
  return task;
}

}  // namespace dom
