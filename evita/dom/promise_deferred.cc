// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/promise_deferred.h"

namespace dom {

PromiseDeferred::PromiseDeferred(v8_glue::Runner* runner)
    : deferred_(runner->isolate(),
                v8::Promise::Resolver::New(runner->isolate())),
      runner_(runner->GetWeakPtr()) {
}

PromiseDeferred::~PromiseDeferred() {
}

void PromiseDeferred::DoReject(v8::Handle<v8::Value> value) {
  DCHECK(runner_);
  auto const isolate = runner_->isolate();
  auto const deferred = deferred_.NewLocal(isolate);
  deferred->Reject(value);
}

void PromiseDeferred::DoResolve(v8::Handle<v8::Value> value) {
  DCHECK(runner_);
  auto const isolate = runner_->isolate();
  auto const deferred = deferred_.NewLocal(isolate);
  deferred->Resolve(value);
}

v8::Local<v8::Promise> PromiseDeferred::GetPromise(v8::Isolate* isolate) const {
  auto const deferred = deferred_.NewLocal(isolate);
  return deferred->GetPromise();
}

}  // namespace dom
