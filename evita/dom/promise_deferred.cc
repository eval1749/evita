// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/promise_deferred.h"

#include "evita/v8_glue/scoped_persistent.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// PromiseDeferred::DeferredImpl
//
class PromiseDeferred::DeferredImpl {
  private: v8_glue::ScopedPersistent<v8::Promise::Resolver> deferred_;

  public: DeferredImpl(v8_glue::Runner* runner);
  public: ~DeferredImpl();

  public: v8::Local<v8::Promise> GetPromise(v8::Isolate* isolate) const;
  public: void Reject(v8_glue::Runner* runner, v8::Handle<v8::Value> reason);
  public: void Resolve(v8_glue::Runner* runner, v8::Handle<v8::Value> value);

  DISALLOW_COPY_AND_ASSIGN(DeferredImpl);
};

PromiseDeferred::DeferredImpl::DeferredImpl(v8_glue::Runner* runner)
    : deferred_(runner->isolate(),
                v8::Promise::Resolver::New(runner->isolate())) {
}

PromiseDeferred::DeferredImpl::~DeferredImpl() {
}

v8::Local<v8::Promise> PromiseDeferred::DeferredImpl::GetPromise(
    v8::Isolate* isolate) const {
  auto const deferred = deferred_.NewLocal(isolate);
  return deferred->GetPromise();
}

void PromiseDeferred::DeferredImpl::Reject(v8_glue::Runner* runner,
                                           v8::Handle<v8::Value> reason) {
  auto const isolate = runner->isolate();
  auto const deferred = deferred_.NewLocal(isolate);
  deferred->Reject(reason);
}

void PromiseDeferred::DeferredImpl::Resolve(v8_glue::Runner* runner,
                                            v8::Handle<v8::Value> value) {
  auto const isolate = runner->isolate();
  auto const deferred = deferred_.NewLocal(isolate);
  deferred->Resolve(value);
}

//////////////////////////////////////////////////////////////////////
//
// PromiseDeferred
//
PromiseDeferred::PromiseDeferred(v8_glue::Runner* runner)
    : deferred_(new DeferredImpl(runner)), runner_(runner->GetWeakPtr()) {
}

PromiseDeferred::~PromiseDeferred() {
}

void PromiseDeferred::DoReject(v8::Handle<v8::Value> reason) {
  DCHECK(runner_);
  deferred_->Reject(runner_.get(), reason);
}

void PromiseDeferred::DoResolve(v8::Handle<v8::Value> reason) {
  DCHECK(runner_);
  deferred_->Resolve(runner_.get(), reason);
}

v8::Local<v8::Promise> PromiseDeferred::GetPromise(v8::Isolate* isolate) const {
  return deferred_->GetPromise(isolate);
}

}  // namespace dom
