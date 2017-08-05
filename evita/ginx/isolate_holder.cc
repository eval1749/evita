// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/ginx/isolate_holder.h"

#include "base/threading/thread_task_runner_handle.h"
#include "evita/ginx/per_isolate_data.h"

namespace ginx {

IsolateHolder::IsolateHolder()
    : gin::IsolateHolder(base::ThreadTaskRunnerHandle::Get()),
      isolate_data_(new PerIsolateData(isolate())) {}

IsolateHolder::~IsolateHolder() = default;

}  // namespace ginx
