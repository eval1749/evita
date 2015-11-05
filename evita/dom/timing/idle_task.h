// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_TIMING_IDLE_TASK_H_
#define EVITA_DOM_TIMING_IDLE_TASK_H_

#include "base/callback.h"
#include "base/location.h"
#include "base/time/time.h"
#include "base/tracking_info.h"

namespace dom {

class IdleDeadlineProvider;

//////////////////////////////////////////////////////////////////////
//
// IdleTask
//
class IdleTask final : public base::TrackingInfo {
 public:
  IdleTask(const tracked_objects::Location& posted_from,
           const base::Closure& callback,
           base::TimeTicks delayed_run_time);
  IdleTask(const tracked_objects::Location& posted_from,
           const base::Closure& callback);
  ~IdleTask();

// Used to support sorting.
  bool operator<(const IdleTask& other) const;

  int id() const { return sequence_num_; }

  void Cancel() { is_canceled_ = true; }
  bool IsCanceled() const { return is_canceled_; }
  void Run();

 private:
  base::Closure callback_;
  int const sequence_num_;
  bool is_canceled_;
};

}  // namespace dom

#endif  // EVITA_DOM_TIMING_IDLE_TASK_H_
