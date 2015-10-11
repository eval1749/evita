// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_SCHEDULER_H_
#define EVITA_DOM_SCHEDULER_H_

#include <memory>
#include <queue>

#include "base/callback.h"
#include "base/macros.h"
#include "common/maybe.h"

namespace base {
class Lock;
class Time;
}

namespace dom {

class ScriptHost;

class Scheduler {
 public:
  Scheduler();
  ~Scheduler();

  void DidBeginFrame(const base::Time& deadline);
  void ScheduleTask(const base::Closure& task);

 private:
  common::Maybe<base::Closure> Take();

  std::unique_ptr<base::Lock> lock_;
  std::queue<base::Closure> task_queue_;

  DISALLOW_COPY_AND_ASSIGN(Scheduler);
};

}  // namespace dom

#endif  // EVITA_DOM_SCHEDULER_H_
