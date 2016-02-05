// Copyright (c) 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/editor.h"

#include "evita/dom/bindings/ginx_IdleRequestOptions.h"
#include "evita/dom/lock.h"
#include "evita/dom/scheduler.h"
#include "evita/dom/script_host.h"
#include "evita/dom/timing/idle_deadline.h"
#include "evita/dom/timing/idle_task.h"
#include "evita/ginx/runner.h"
#include "evita/ginx/scoped_persistent.h"

namespace dom {

namespace {

v8::Eternal<v8::Value> idle_deadline_object;

//////////////////////////////////////////////////////////////////////
//
// IdleTaskWrapper
//
class IdleTaskWrapper {
 public:
  IdleTaskWrapper(v8::Isolate* isolate, v8::Local<v8::Function> callback);
  ~IdleTaskWrapper() = default;

  void Run();

 private:
  ginx::ScopedPersistent<v8::Function> callback_;

  DISALLOW_COPY_AND_ASSIGN(IdleTaskWrapper);
};

IdleTaskWrapper::IdleTaskWrapper(v8::Isolate* isolate,
                                 v8::Local<v8::Function> callback) {
  callback_.Reset(isolate, callback);
}

void IdleTaskWrapper::Run() {
  auto const runner = ScriptHost::instance()->runner();
  auto const isolate = runner->isolate();
  ginx::Runner::Scope runner_scope(runner);
  if (idle_deadline_object.IsEmpty()) {
    idle_deadline_object.Set(isolate,
                             gin::ConvertToV8(isolate, new IdleDeadline()));
  }
  DOM_AUTO_LOCK_SCOPE();
  runner->CallAsFunction(callback_.NewLocal(isolate), v8::Undefined(isolate),
                         idle_deadline_object.Get(isolate));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Editor
//

void Editor::CancelIdleCallback(int handle) {
  ScriptHost::instance()->scheduler()->CancelIdleTask(handle);
}

int Editor::RequestIdleCallback(v8::Local<v8::Function> callback,
                                const IdleRequestOptions& options) {
  auto const timeout = options.timeout();
  auto const isolate = ScriptHost::instance()->isolate();
  auto const run_time =
      timeout
          ? base::TimeTicks::Now() + base::TimeDelta::FromMilliseconds(timeout)
          : base::TimeTicks();
  return ScriptHost::instance()->scheduler()->ScheduleIdleTask(
      IdleTask(FROM_HERE,
               base::Bind(&IdleTaskWrapper::Run,
                          base::Owned(new IdleTaskWrapper(isolate, callback))),
               run_time));
}

int Editor::RequestIdleCallback(v8::Local<v8::Function> callback) {
  IdleRequestOptions options;
  options.set_timeout(0);
  return RequestIdleCallback(callback, options);
}

}  // namespace dom
