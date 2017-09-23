// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_SCHEDULER_IDLE_TASK_H_
#define EVITA_DOM_SCHEDULER_IDLE_TASK_H_

#include "base/callback.h"
#include "base/location.h"
#include "base/time/time.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// IdleTask
//
class IdleTask final {
 public:
  using Callback = base::Callback<void(const base::TimeTicks&)>;

  IdleTask(const base::Location& posted_from,
           const Callback& callback,
           base::TimeTicks delayed_run_time);
  IdleTask(const base::Location& posted_from, const Callback& callback);
  IdleTask(const IdleTask&);
  ~IdleTask();

  // Used to support sorting.
  bool operator<(const IdleTask& other) const;

  base::TimeTicks delayed_run_time() const { return delayed_run_time_; }
  int id() const { return sequence_num_; }

  void Cancel() { is_canceled_ = true; }
  bool IsCanceled() const { return is_canceled_; }
  void Run(const base::TimeTicks& deadline);

 private:
  Callback callback_;

  // The time when the task should be run.
  base::TimeTicks delayed_run_time_;

  // The site this PendingTask was posted from.
  base::Location posted_from_;

  int const sequence_num_;
  bool is_canceled_ = false;
};

}  // namespace dom

#endif  // EVITA_DOM_SCHEDULER_IDLE_TASK_H_
