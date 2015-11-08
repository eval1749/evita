// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PROMISE_RESOLVER_H_
#define EVITA_DOM_PROMISE_RESOLVER_H_

#include "base/location.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "evita/dom/public/deferred.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/runner.h"
#include "evita/v8_glue/scoped_persistent.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// PromiseResolver
// The instance of |PromiseResolver| keeps a JavaScript object of
// |Promise.defer()| during asynchronous call.
//
class PromiseResolver final
    : public base::RefCountedThreadSafe<PromiseResolver> {
 public:
  ~PromiseResolver();

  template <typename ResolveType, typename RejectType>
  static v8::Handle<v8::Promise> Call(
      const tracked_objects::Location& from_here,
      const base::Callback<
          void(const domapi::Deferred<ResolveType, RejectType>&)> closure);

  v8::Local<v8::Promise> GetPromise(v8::Isolate* isolate) const;

  template <typename T>
  void Reject(T reason);

  template <typename T>
  void Resolve(T value);

 private:
  PromiseResolver(const tracked_objects::Location& from_here,
                  v8_glue::Runner* runner);

  v8_glue::Runner* runner() const { return runner_.get(); }

  void DoReject(v8::Handle<v8::Value> reason);
  void DoResolve(v8::Handle<v8::Value> value);

  tracked_objects::Location from_here_;
  v8_glue::ScopedPersistent<v8::Promise::Resolver> resolver_;
  base::WeakPtr<v8_glue::Runner> runner_;
  int const sequence_num_;

  DISALLOW_COPY_AND_ASSIGN(PromiseResolver);
};

template <typename T, typename U>
v8::Handle<v8::Promise> PromiseResolver::Call(
    const tracked_objects::Location& from_here,
    const base::Callback<void(const domapi::Deferred<T, U>&)> closure) {
  auto const runner = ScriptHost::instance()->runner();
  v8_glue::Runner::EscapableHandleScope runner_scope(runner);

  auto const resolver =
      make_scoped_refptr(new PromiseResolver(from_here, runner));

  domapi::Deferred<T, U> deferred;
  deferred.reject = base::Bind(&PromiseResolver::Reject<U>, resolver);
  deferred.resolve = base::Bind(&PromiseResolver::Resolve<T>, resolver);
  deferred.sequence_num = resolver->sequence_num_;
  closure.Run(deferred);
  return runner_scope.Escape(resolver->GetPromise(runner->isolate()));
}

template <typename T>
void PromiseResolver::Reject(T reason) {
  if (!runner_)
    return;
  v8_glue::Runner::Scope runner_scope(runner_.get());
  auto const isolate = runner_->isolate();
  v8::Local<v8::Value> v8_value;
  if (!gin::TryConvertToV8(isolate, reason, &v8_value))
    return;
  DoReject(v8_value);
}

template <typename T>
void PromiseResolver::Resolve(T value) {
  if (!runner_)
    return;
  v8_glue::Runner::Scope runner_scope(runner_.get());
  auto const isolate = runner_->isolate();
  v8::Local<v8::Value> v8_value;
  if (!gin::TryConvertToV8(isolate, value, &v8_value))
    return;
  DoResolve(v8_value);
}

}  // namespace dom

#endif  // EVITA_DOM_PROMISE_RESOLVER_H_
