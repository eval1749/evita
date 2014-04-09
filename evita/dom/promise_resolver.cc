// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/promise_resolver.h"

#pragma warning(push)
#pragma warning(disable: 4100 4625 4626)
#include "base/bind.h"
#include "base/callback.h"
#include "base/message_loop/message_loop.h"
#pragma warning(pop)
#include "evita/dom/script_host.h"

namespace dom {

PromiseResolver::PromiseResolver(v8_glue::Runner* runner)
    : resolver_(runner->isolate(),
                v8::Promise::Resolver::New(runner->isolate())),
      runner_(runner->GetWeakPtr()) {
}

PromiseResolver::~PromiseResolver() {
}

void PromiseResolver::DoReject(v8::Handle<v8::Value> value) {
  DCHECK(runner_);
  auto const isolate = runner_->isolate();
  auto const resolver = resolver_.NewLocal(isolate);
  resolver->Reject(value);
}

void PromiseResolver::DoResolve(v8::Handle<v8::Value> value) {
  DCHECK(runner_);
  auto const isolate = runner_->isolate();
  auto const resolver = resolver_.NewLocal(isolate);
  resolver->Resolve(value);
}

v8::Local<v8::Promise> PromiseResolver::GetPromise(v8::Isolate* isolate) const {
  auto const resolver = resolver_.NewLocal(isolate);
  return resolver->GetPromise();
}

void PromiseResolver::ScheduleRunMicrotasks() {
  base::MessageLoop::current()->PostTask(FROM_HERE, base::Bind(
      &ScriptHost::RunMicrotasks, base::Unretained(ScriptHost::instance())));
}

}  // namespace dom
