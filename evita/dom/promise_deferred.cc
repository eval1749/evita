// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "evita/dom/promise_deferred.h"

#include "evita/v8_glue/scoped_persistent.h"

namespace dom {

#define DEFINE_STATIC_V8_STRING(name) \
  CR_DEFINE_STATIC_LOCAL(v8::Eternal<v8::String>, name, \
                         (isolate, NewString(isolate, #name)))

namespace {

static v8::Local<v8::String> NewString(v8::Isolate* isolate,
                                        const char* string) {
  return v8::String::NewFromOneByte(isolate,
                                    reinterpret_cast<const uint8_t*>(string));
}

v8::Local<v8::Object> NewDeferred(v8_glue::Runner* runner) {
  auto const isolate = runner->isolate();
  v8::EscapableHandleScope handle_scope(isolate);
  DEFINE_STATIC_V8_STRING(Promise);
  DEFINE_STATIC_V8_STRING(defer);
  auto const promise = runner->global()->Get(Promise.Get(isolate))->ToObject();
  auto const promise_defer = promise->Get(defer.Get(isolate));
  DCHECK(!promise_defer.IsEmpty() && promise_defer->IsFunction());
  auto const defered = runner->Call(promise_defer, promise);
  DCHECK(!defered.IsEmpty() && defered->IsObject());
  return handle_scope.Escape(defered->ToObject());
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// PromiseDeferred::DeferredImpl
//
class PromiseDeferred::DeferredImpl {
  private: v8_glue::ScopedPersistent<v8::Object> deferred_;

  public: DeferredImpl(v8_glue::Runner* runner);
  public: ~DeferredImpl();

  public: v8::Local<v8::Object> GetPromise(v8::Isolate* isoalte) const;
  public: void Reject(v8_glue::Runner* runner, v8::Handle<v8::Value> reason);
  public: void Resolve(v8_glue::Runner* runner, v8::Handle<v8::Value> value);

  DISALLOW_COPY_AND_ASSIGN(DeferredImpl);
};

PromiseDeferred::DeferredImpl::DeferredImpl(v8_glue::Runner* runner)
    : deferred_(runner->isolate(), NewDeferred(runner)) {
}

PromiseDeferred::DeferredImpl::~DeferredImpl() {
}

v8::Local<v8::Object> PromiseDeferred::DeferredImpl::GetPromise(
    v8::Isolate* isolate) const {
  auto const deferred = deferred_.NewLocal(isolate);
  DEFINE_STATIC_V8_STRING(promise);
  return v8::Local<v8::Object>(deferred->Get(promise.Get(isolate))->ToObject());
}

void PromiseDeferred::DeferredImpl::Reject(v8_glue::Runner* runner,
                                    v8::Handle<v8::Value> reason) {
  auto const isolate = runner->isolate();
  DEFINE_STATIC_V8_STRING(reject);
  auto const deferred = deferred_.NewLocal(isolate);
  runner->Call(deferred->Get(reject.Get(isolate))->ToObject(),
               v8::Undefined(isolate), reason);
}

void PromiseDeferred::DeferredImpl::Resolve(v8_glue::Runner* runner,
                                     v8::Handle<v8::Value> value) {
  auto const isolate = runner->isolate();
  DEFINE_STATIC_V8_STRING(resolve);
  auto const deferred = deferred_.NewLocal(isolate);
  runner->Call(deferred->Get(resolve.Get(isolate)),
               v8::Undefined(isolate), value);
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

v8::Local<v8::Object> PromiseDeferred::GetPromise(v8::Isolate* isolate) const {
  return deferred_->GetPromise(isolate);
}

}  // namespace dom

namespace gin {
v8::Handle<v8::Value> Converter<dom::PromiseDeferred*>::ToV8(
    v8::Isolate* isolate, dom::PromiseDeferred* promise_deferred) {
  return promise_deferred->GetPromise(isolate);
}
}   // namespace gin
