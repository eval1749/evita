// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_SCHEDULER_H_
#define EVITA_DOM_SCHEDULER_H_

#include "base/callback.h"
#include "base/macros.h"

namespace base {
class Time;
}

namespace dom {

class Scheduler {
 public:
  virtual ~Scheduler();

  virtual void DidBeginFrame(const base::Time& deadline) = 0;
  virtual void DidEnterViewIdle(const base::Time& deadline) = 0;
  virtual void DidExitViewIdle() = 0;
  virtual void RunIdleTasks() = 0;
  virtual void ScheduleTask(const base::Closure& task) = 0;
  virtual void ScheduleIdleTask(const base::Closure& task) = 0;

 protected:
  Scheduler();

 private:
  DISALLOW_COPY_AND_ASSIGN(Scheduler);
};

}  // namespace dom

#endif  // EVITA_DOM_SCHEDULER_H_
