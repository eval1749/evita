// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_SCHEDULER_H_
#define EVITA_DOM_SCHEDULER_H_

#include <memory>

#include "base/callback.h"
#include "base/macros.h"

namespace base {
class TimeTicks;
}

namespace dom {

class AnimationFrameCallback;
class IdleDeadlineProvider;
class IdleTask;

class Scheduler {
 public:
  virtual ~Scheduler();

  virtual void CancelAnimationFrame(int request_id) = 0;
  virtual void CancelIdleTask(int task_id) = 0;
  virtual void DidBeginFrame(const base::TimeTicks& deadline) = 0;
  virtual void DidEnterViewIdle(const base::TimeTicks& deadline) = 0;
  virtual void DidExitViewIdle() = 0;
  virtual IdleDeadlineProvider* GetIdleDeadlineProvider() = 0;
  virtual int RequestAnimationFrame(
      std::unique_ptr<AnimationFrameCallback> callback) = 0;
  virtual void RunIdleTasks() = 0;
  virtual int ScheduleIdleTask(const IdleTask& task) = 0;
  virtual void ScheduleTask(const base::Closure& task) = 0;

 protected:
  Scheduler();

 private:
  DISALLOW_COPY_AND_ASSIGN(Scheduler);
};

}  // namespace dom

#endif  // EVITA_DOM_SCHEDULER_H_
