// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "evita/dom/scheduler/micro_task.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// MicroTask
//
MicroTask::MicroTask(const base::Location& posted_from,
                     const base::Closure& callback)
    : callback_(callback), posted_from_(posted_from) {}

MicroTask::~MicroTask() = default;

void MicroTask::Run(void* data) {
  std::unique_ptr<MicroTask> task(static_cast<MicroTask*>(data));
  task->callback_.Run();
}

}  // namespace dom
