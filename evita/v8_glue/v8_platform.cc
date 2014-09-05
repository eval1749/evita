// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/v8_glue/v8_platform.h"

#pragma warning(push)
#pragma warning(disable: 4100 4625 4626)
#include "base/bind.h"
#include "base/location.h"
#include "base/message_loop/message_loop_proxy.h"
#include "base/threading/worker_pool.h"
#pragma warning(pop)
#include "evita/v8_glue/per_isolate_data.h"

namespace v8_glue {

V8Platform::V8Platform() {
}

V8Platform::~V8Platform() {
}

void V8Platform::CallOnBackgroundThread(
    v8::Task* task, v8::Platform::ExpectedRuntime expected_runtime) {
  base::WorkerPool::PostTask(
      FROM_HERE,
      base::Bind(&v8::Task::Run, base::Owned(task)),
      expected_runtime == v8::Platform::kLongRunningTask);
}

void V8Platform::CallOnForegroundThread(v8::Isolate* isolate, v8::Task* task) {
  PerIsolateData::From(isolate)->message_loop_proxy()->PostTask(
      FROM_HERE, base::Bind(&v8::Task::Run, base::Owned(task)));
}

}  // namespace v8_glue
