// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/timing/idle_task.h"

#include "base/tracked_objects.h"

namespace dom {

static int current_sequence_num;

IdleTask::IdleTask(const tracked_objects::Location& posted_from,
                   const base::Closure& callback,
                   base::TimeTicks delay_run_time)
    : base::TrackingInfo(posted_from, delay_run_time),
      callback_(callback),
      is_canceled_(false),
      sequence_num_(++current_sequence_num) {}

IdleTask::IdleTask(const tracked_objects::Location& posted_from,
                   const base::Closure& callback)
    : IdleTask(posted_from, callback, base::TimeTicks()) {}

IdleTask::~IdleTask() {}


bool IdleTask::operator<(const IdleTask& other) const {
  // Since the top of a priority queue is defined as the "greatest" element, we
  // need to invert the comparison here.  We want the smaller time to be at the
  // top of the heap.

  if (delayed_run_time < other.delayed_run_time)
    return false;

  if (delayed_run_time > other.delayed_run_time)
    return true;

  // If the times happen to match, then we use the sequence number to decide.
  // Compare the difference to support integer roll-over.
  return (sequence_num_ - other.sequence_num_) > 0;
}

void IdleTask::Run() {
  callback_.Run();
}

}  // namespace dom
