// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_SCHEDULER_MICRO_TASK_H_
#define EVITA_DOM_SCHEDULER_MICRO_TASK_H_

#include "base/callback.h"
#include "base/location.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// MicroTask
//
class MicroTask final {
 public:
  MicroTask(const base::Location& posted_from, base::OnceClosure callback);
  ~MicroTask();

  static void Run(void* data);

 private:
  base::OnceClosure callback_;
  base::Location posted_from_;

  DISALLOW_COPY_AND_ASSIGN(MicroTask);
};

}  // namespace dom

#endif  // EVITA_DOM_SCHEDULER_MICRO_TASK_H_
