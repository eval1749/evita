// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EVITA_DOM_PROMISE_RESOLVER_H_
#define EVITA_DOM_PROMISE_RESOLVER_H_

#include "base/location.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "evita/dom/public/promise.h"
#include "evita/ginx/converter.h"
#include "evita/ginx/runner.h"
#include "evita/ginx/scoped_persistent.h"

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
  static v8::Local<v8::Promise> Call(
      const tracked_objects::Location& from_here,
      const base::Callback<
          void(const domapi::Promise<ResolveType, RejectType>&)> closure);

  v8::Local<v8::Promise> GetPromise(v8::Isolate* isolate) const;

  template <typename T>
  void Reject(T reason);

  template <typename T>
  void Resolve(T value);

 private:
  PromiseResolver(const tracked_objects::Location& from_here,
                  ginx::Runner* runner);

  ginx::Runner* runner() const {
    return reinterpret_cast<ginx::Runner*>(runner_.get());
  }

  void DoReject(v8::Local<v8::Value> reason);
  void DoResolve(v8::Local<v8::Value> value);

  tracked_objects::Location from_here_;
  ginx::ScopedPersistent<v8::Promise::Resolver> resolver_;
  base::WeakPtr<gin::Runner> runner_;
  int const sequence_num_;

  DISALLOW_COPY_AND_ASSIGN(PromiseResolver);
};

template <typename T, typename U>
v8::Local<v8::Promise> PromiseResolver::Call(
    const tracked_objects::Location& from_here,
    const base::Callback<void(const domapi::Promise<T, U>&)> closure) {
  const auto runner = ScriptHost::instance()->runner();
  ginx::Runner::EscapableHandleScope runner_scope(runner);

  const auto& resolver =
      make_scoped_refptr(new PromiseResolver(from_here, runner));

  domapi::Promise<T, U> promise;
  promise.reject = base::Bind(&PromiseResolver::Reject<U>, resolver);
  promise.resolve = base::Bind(&PromiseResolver::Resolve<T>, resolver);
  promise.sequence_num = resolver->sequence_num_;
  closure.Run(promise);
  return runner_scope.Escape(resolver->GetPromise(runner->isolate()));
}

template <typename T>
void PromiseResolver::Reject(T reason) {
  if (!runner_)
    return;
  ginx::Runner::Scope runner_scope(runner());
  const auto isolate = runner()->isolate();
  v8::Local<v8::Value> v8_value;
  if (!gin::TryConvertToV8(isolate, reason, &v8_value))
    return;
  DoReject(v8_value);
}

template <typename T>
void PromiseResolver::Resolve(T value) {
  if (!runner_)
    return;
  ginx::Runner::Scope runner_scope(runner());
  const auto isolate = runner()->isolate();
  v8::Local<v8::Value> v8_value;
  if (!gin::TryConvertToV8(isolate, value, &v8_value))
    return;
  DoResolve(v8_value);
}

}  // namespace dom

#endif  // EVITA_DOM_PROMISE_RESOLVER_H_
