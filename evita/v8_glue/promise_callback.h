// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_evita_v8_glue_promise_callback_h)
#define INCLUDE_evita_v8_glue_promise_callback_h

#include <memory>

#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/runner.h"

namespace v8_glue {

class PromiseCallback : public base::RefCounted<PromiseCallback> {
  private: class Deferred;

  private: std::unique_ptr<Deferred> deferred_;
  private: base::WeakPtr<v8_glue::Runner> runner_;

  protected: PromiseCallback(v8_glue::Runner* runner);
  public: ~PromiseCallback();

  protected: Runner* runner() const { return runner_.get(); }

  private: void DoReject(v8::Handle<v8::Value> reason);
  private: void DoResolve(v8::Handle<v8::Value> value);
  public: v8::Local<v8::Object> GetPromise(v8::Isolate* isoalte) const;
  protected: template<typename T> void Reject(T reason);
  protected: template<typename T> void Resolve(T value);

  DISALLOW_COPY_AND_ASSIGN(PromiseCallback);
};

template<typename T>
void PromiseCallback::Reject(T reason) {
  if (!runner_)
    return;
  Runner::Scope runner_scope(runner_.get());
  DoReject(gin::ConvertToV8(runner_->isolate(), reason));
}

template<typename T>
void PromiseCallback::Resolve(T value) {
  if (!runner_)
    return;
  Runner::Scope runner_scope(runner_.get());
  DoResolve(gin::ConvertToV8(runner_->isolate(), value));
}

}  // namespace v8_glue

#endif //!defined(INCLUDE_evita_v8_glue_promise_callback_h)
