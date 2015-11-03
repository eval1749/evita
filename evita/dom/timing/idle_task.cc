// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/timing/idle_task.h"

#include "base/tracked_objects.h"

namespace dom {

static int current_task_id;

IdleTask::IdleTask(const tracked_objects::Location& posted_from,
                   const base::Closure& callback,
                   base::TimeTicks delay_run_time)
    : base::TrackingInfo(posted_from, delay_run_time),
      callback_(callback),
      id_(++current_task_id),
      is_canceled_(false) {}

IdleTask::IdleTask(const tracked_objects::Location& posted_from,
                   const base::Closure& callback)
    : IdleTask(posted_from, callback, base::TimeTicks()) {}

IdleTask::~IdleTask() {}

void IdleTask::Run() {
  callback_.Run();
}

}  // namespace dom
