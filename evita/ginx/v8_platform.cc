// Copyright (c) 1996-2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/ginx/v8_platform.h"

#include "base/bind.h"
#include "base/lazy_instance.h"
#include "base/location.h"
#include "base/task_runner.h"
#include "base/task_scheduler/post_task.h"
#include "evita/ginx/per_isolate_data.h"

namespace ginx {

namespace {

constexpr base::TaskTraits kBackgroundThreadTaskTraits = {
    base::TaskPriority::USER_VISIBLE};

base::LazyInstance<V8Platform>::Leaky g_v8_platform = LAZY_INSTANCE_INITIALIZER;

void RunWithLocker(v8::Isolate* isolate, v8::Task* task) {
  v8::Locker lock(isolate);
  task->Run();
}

}  // namespace

V8Platform::V8Platform() {}

V8Platform::~V8Platform() {}

void V8Platform::CallOnBackgroundThread(
    v8::Task* task,
    v8::Platform::ExpectedRuntime expected_runtime) {
  base::PostTaskWithTraits(FROM_HERE, kBackgroundThreadTaskTraits,
                           base::BindOnce(&v8::Task::Run, base::Owned(task)));
}

void V8Platform::CallOnForegroundThread(v8::Isolate* isolate, v8::Task* task) {
  PerIsolateData::From(isolate)->task_runner()->PostTask(
      FROM_HERE, base::BindOnce(&v8::Task::Run, base::Owned(task)));
}

}  // namespace ginx
