// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/timing/idle_task.h"

#include "base/tracked_objects.h"

namespace dom {

IdleTask::IdleTask(const tracked_objects::Location& posted_from,
                   const Callback& callback,
                   base::TimeTicks delay_run_time)
    : base::TrackingInfo(posted_from, delay_run_time),
      callback_(callback),
      is_canceled_(false) {}

IdleTask::IdleTask(const tracked_objects::Location& posted_from,
                   const Callback& callback)
    : IdleTask(posted_from, callback, base::TimeTicks()) {}

IdleTask::~IdleTask() {}

void IdleTask::Run(IdleDeadlineProvider* idle_deadline) {
  callback_.Run(idle_deadline);
}

}  // namespace dom
