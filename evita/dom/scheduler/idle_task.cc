// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/scheduler/idle_task.h"

#include "base/tracked_objects.h"

namespace dom {

static int current_sequence_num;

IdleTask::IdleTask(const tracked_objects::Location& posted_from,
                   const Callback& callback,
                   base::TimeTicks delay_run_time)
    : callback_(callback),
      delayed_run_time_(delay_run_time),
      posted_from_(posted_from),
      sequence_num_(++current_sequence_num) {}

IdleTask::IdleTask(const tracked_objects::Location& posted_from,
                   const Callback& callback)
    : IdleTask(posted_from, callback, base::TimeTicks()) {}

IdleTask::IdleTask(const IdleTask&) = default;
IdleTask::~IdleTask() = default;

bool IdleTask::operator<(const IdleTask& other) const {
  // Since the top of a priority queue is defined as the "greatest" element, we
  // need to invert the comparison here.  We want the smaller time to be at the
  // top of the heap.

  if (delayed_run_time_ < other.delayed_run_time_)
    return false;

  if (delayed_run_time_ > other.delayed_run_time_)
    return true;

  // If the times happen to match, then we use the sequence number to decide.
  // Compare the difference to support integer roll-over.
  return (sequence_num_ - other.sequence_num_) > 0;
}

void IdleTask::Run(const base::TimeTicks& deadline) {
  callback_.Run(deadline);
}

}  // namespace dom
