// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_set>

#include "evita/dom/promise_resolver.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/message_loop/message_loop.h"
#include "base/trace_event/trace_event.h"
#include "common/memory/singleton.h"

namespace dom {

namespace {
class LivePromiseResolverSet final
    : public common::Singleton<LivePromiseResolverSet> {
  DECLARE_SINGLETON_CLASS(LivePromiseResolverSet);

 public:
  ~LivePromiseResolverSet() final = default;

  bool IsLive(const PromiseResolver* resolver) const;
  void Register(const PromiseResolver* resolver);
  void Unregister(const PromiseResolver* resolver);

 private:
  LivePromiseResolverSet() = default;

  std::unordered_set<const PromiseResolver*> lives_;

  DISALLOW_COPY_AND_ASSIGN(LivePromiseResolverSet);
};

bool LivePromiseResolverSet::IsLive(const PromiseResolver* resolver) const {
  return lives_.count(resolver) == 1;
}

void LivePromiseResolverSet::Register(const PromiseResolver* resolver) {
  lives_.insert(resolver);
}

void LivePromiseResolverSet::Unregister(const PromiseResolver* resolver) {
  lives_.erase(lives_.find(resolver));
}

// Sequence number for trace events.
int promise_sequence_num;

}  // namespace

PromiseResolver::PromiseResolver(const tracked_objects::Location& from_here,
                                 ginx::Runner* runner)
    : from_here_(from_here),
      resolver_(runner->isolate(),
                v8::Promise::Resolver::New(runner->context())
                    .FromMaybe(v8::Local<v8::Promise::Resolver>())),
      runner_(runner->GetWeakPtr()),
      sequence_num_(++promise_sequence_num) {
  TRACE_EVENT_ASYNC_BEGIN1("script", "Promise", sequence_num_, "function",
                           from_here.function_name());
  TRACE_EVENT_WITH_FLOW1("promise", "Promise", sequence_num_,
                         TRACE_EVENT_FLAG_FLOW_OUT, "function",
                         from_here.function_name());
  LivePromiseResolverSet::instance()->Register(this);
}

PromiseResolver::~PromiseResolver() {
  LivePromiseResolverSet::instance()->Unregister(this);
}

void PromiseResolver::DoReject(v8::Local<v8::Value> value) {
  TRACE_EVENT_WITH_FLOW1("promise", "PromiseResolver::Reject", sequence_num_,
                         TRACE_EVENT_FLAG_FLOW_IN, "function",
                         from_here_.function_name());
  CHECK(LivePromiseResolverSet::instance()->IsLive(this));
  CHECK(runner_);
  if (!resolver_.IsEmpty()) {
    const auto& isolate = runner()->isolate();
    const auto& resolver = resolver_.NewLocal(isolate);
    const auto& result = resolver->Reject(runner()->context(), value);
    CHECK(result.IsJust());
  }
  TRACE_EVENT_ASYNC_END1("script", "Promise", sequence_num_, "type", "reject");
}

void PromiseResolver::DoResolve(v8::Local<v8::Value> value) {
  TRACE_EVENT_WITH_FLOW1("promise", "PromiseResolver::Resolver", sequence_num_,
                         TRACE_EVENT_FLAG_FLOW_IN, "function",
                         from_here_.function_name());
  CHECK(LivePromiseResolverSet::instance()->IsLive(this));
  CHECK(runner_);
  if (!resolver_.IsEmpty()) {
    const auto& isolate = runner()->isolate();
    const auto& resolver = resolver_.NewLocal(isolate);
    const auto& result = resolver->Resolve(runner()->context(), value);
    CHECK(result.IsJust());
  }
  TRACE_EVENT_ASYNC_END1("script", "Promise", sequence_num_, "type", "resolve");
}

v8::Local<v8::Promise> PromiseResolver::GetPromise(v8::Isolate* isolate) const {
  CHECK(LivePromiseResolverSet::instance()->IsLive(this));
  const auto& resolver = resolver_.NewLocal(isolate);
  return resolver->GetPromise();
}

}  // namespace dom
