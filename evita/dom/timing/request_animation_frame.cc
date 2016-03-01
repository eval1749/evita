// Copyright (c) 2016 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/editor.h"

#include "base/time/time.h"
#include "evita/dom/lock.h"
#include "evita/dom/scheduler/animation_frame_callback.h"
#include "evita/dom/scheduler/scheduler.h"
#include "evita/dom/script_host.h"
#include "evita/ginx/runner.h"
#include "evita/ginx/scoped_persistent.h"

namespace dom {

namespace {

//////////////////////////////////////////////////////////////////////
//
// FrameRequestCallbackWrapper
//
class FrameRequestCallbackWrapper {
 public:
  FrameRequestCallbackWrapper(v8::Isolate* isolate,
                              v8::Local<v8::Function> callback);
  ~FrameRequestCallbackWrapper() = default;

  void Run(const base::TimeTicks& time);

 private:
  ginx::ScopedPersistent<v8::Function> callback_;

  DISALLOW_COPY_AND_ASSIGN(FrameRequestCallbackWrapper);
};

FrameRequestCallbackWrapper::FrameRequestCallbackWrapper(
    v8::Isolate* isolate,
    v8::Local<v8::Function> callback) {
  callback_.Reset(isolate, callback);
}

void FrameRequestCallbackWrapper::Run(const base::TimeTicks& time) {
  auto const runner = ScriptHost::instance()->runner();
  auto const isolate = runner->isolate();
  ginx::Runner::Scope runner_scope(runner);
  DOM_AUTO_LOCK_SCOPE();
  runner->CallAsFunction(
      callback_.NewLocal(isolate), v8::Undefined(isolate),
      gin::ConvertToV8(isolate, (time - base::TimeTicks()).InMillisecondsF()));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Editor
//

void Editor::CancelAnimationFrame(int handle) {
  ScriptHost::instance()->scheduler()->CancelAnimationFrame(handle);
}

int Editor::RequestAnimationFrame(v8::Local<v8::Function> callback) {
  auto const isolate = ScriptHost::instance()->isolate();
  auto request = std::make_unique<AnimationFrameCallback>(
      FROM_HERE, base::Bind(&FrameRequestCallbackWrapper::Run,
                            base::Owned(new FrameRequestCallbackWrapper(
                                isolate, callback))));
  return ScriptHost::instance()->scheduler()->RequestAnimationFrame(
      std::move(request));
}

}  // namespace dom
