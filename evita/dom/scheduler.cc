// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/scheduler.h"

#include "base/synchronization/lock.h"
#include "evita/dom/public/view_event.h"
#include "evita/dom/script_host.h"

namespace dom {

Scheduler::Scheduler(ScriptHost* script_host)
    : lock_(new base::Lock()), script_host_(script_host) {}

Scheduler::~Scheduler() {}

void Scheduler::DidBeginFrame(const base::Time& deadline) {
  for (;;) {
    auto maybe_task = Take();
    if (maybe_task.IsNothing())
      break;
    maybe_task.FromJust().Run();
  }
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
