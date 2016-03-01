// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_SCHEDULER_IDLE_TASK_QUEUE_H_
#define EVITA_DOM_SCHEDULER_IDLE_TASK_QUEUE_H_

#include <queue>
#include <unordered_map>

#include "base/macros.h"

namespace base {
class TimeTicks;
}

namespace dom {

class IdleTask;

//////////////////////////////////////////////////////////////////////
//
// IdleTaskQueue
//
class IdleTaskQueue final {
 public:
  IdleTaskQueue();
  ~IdleTaskQueue();

  void CancelTask(int task_id);
  int GiveTask(const IdleTask& task);
  void RunIdleTasks(const base::TimeTicks& deadline);

 private:
  void RemoveTask(IdleTask* task);

  std::queue<IdleTask*> ready_tasks_;
  std::unordered_map<int, IdleTask*> task_map_;
  std::priority_queue<IdleTask*> waiting_tasks_;

  DISALLOW_COPY_AND_ASSIGN(IdleTaskQueue);
};

}  // namespace dom

#endif  // EVITA_DOM_SCHEDULER_IDLE_TASK_QUEUE_H_
