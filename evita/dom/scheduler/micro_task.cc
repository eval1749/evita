// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/scheduler/micro_task.h"

#include <memory>
#include <utility>

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// MicroTask
//
MicroTask::MicroTask(const base::Location& posted_from,
                     base::OnceClosure callback)
    : callback_(std::move(callback)), posted_from_(posted_from) {}

MicroTask::~MicroTask() = default;

void MicroTask::Run(void* data) {
  std::unique_ptr<MicroTask> task(static_cast<MicroTask*>(data));
  std::move(task->callback_).Run();
}

}  // namespace dom
