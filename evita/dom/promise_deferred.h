// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_dom_promise_deferred_h)
#define INCLUDE_evita_dom_promise_deferred_h

#include <memory>

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "evita/dom/public/deferred.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/runner.h"

namespace dom {

//////////////////////////////////////////////////////////////////////
//
// PromiseDeferred
// Teh instance of |PromiseDeferred| keeps a JavaScript object of
// |Promise.defer()| during asynchronous call.
//
class PromiseDeferred : public base::RefCounted<PromiseDeferred> {
  private: class DeferredImpl;

  private: std::unique_ptr<DeferredImpl> deferred_;
  private: base::WeakPtr<v8_glue::Runner> runner_;

  public: PromiseDeferred(v8_glue::Runner* runner);
  public: ~PromiseDeferred();

  protected: v8_glue::Runner* runner() const { return runner_.get(); }

  public: template<typename ResolveType, typename RejectType>
    static v8::Handle<v8::Promise> Call(
        const base::Callback<
            void(const domapi::Deferred<ResolveType, RejectType>&)> closure);

  private: void DoReject(v8::Handle<v8::Value> reason);
  private: void DoResolve(v8::Handle<v8::Value> value);
  public: v8::Local<v8::Promise> GetPromise(v8::Isolate* isoalte) const;
  public: template<typename T> void Reject(T reason);
  public: template<typename T> void Resolve(T value);

  DISALLOW_COPY_AND_ASSIGN(PromiseDeferred);
};

template<typename T, typename U>
v8::Handle<v8::Promise> PromiseDeferred::Call(
    const base::Callback<void(const domapi::Deferred<T, U>&)> closure) {
  auto const runner = ScriptHost::instance()->runner();
  v8_glue::Runner::EscapableHandleScope runner_scope(runner);

  auto const promise_deferred =
      make_scoped_refptr(new PromiseDeferred(runner));

  domapi::Deferred<T, U> deferred;
  deferred.reject = base::Bind(&PromiseDeferred::Reject<U>,
                               promise_deferred);
  deferred.resolve = base::Bind(&PromiseDeferred::Resolve<T>,
                                promise_deferred);
  closure.Run(deferred);
  return runner_scope.Escape(promise_deferred->GetPromise(runner->isolate()));
}

template<typename T>
void PromiseDeferred::Reject(T reason) {
  if (!runner_)
    return;
  v8_glue::Runner::Scope runner_scope(runner_.get());
  DoReject(gin::ConvertToV8(runner_->isolate(), reason));
}

template<typename T>
void PromiseDeferred::Resolve(T value) {
  if (!runner_)
    return;
  v8_glue::Runner::Scope runner_scope(runner_.get());
  DoResolve(gin::ConvertToV8(runner_->isolate(), value));
}

}  // namespace dom

#endif //!defined(INCLUDE_evita_dom_promise_deferred_h)
