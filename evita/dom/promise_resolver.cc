// Copyright (c) 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_set>

#include "evita/dom/promise_resolver.h"

#include "base/bind.h"
#include "base/callback.h"
#include "base/message_loop/message_loop.h"
#include "common/memory/singleton.h"
#include "evita/dom/script_host.h"

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

}  // namespace

PromiseResolver::PromiseResolver(Type type, v8_glue::Runner* runner)
    : resolver_(runner->isolate(),
                v8::Promise::Resolver::New(runner->isolate())),
      runner_(runner->GetWeakPtr()),
      type_(type) {
  LivePromiseResolverSet::instance()->Register(this);
}

PromiseResolver::~PromiseResolver() {
  LivePromiseResolverSet::instance()->Unregister(this);
}

void PromiseResolver::DoReject(v8::Handle<v8::Value> value) {
  CHECK(LivePromiseResolverSet::instance()->IsLive(this));
  CHECK(runner_);
  auto const isolate = runner_->isolate();
  auto const resolver = resolver_.NewLocal(isolate);
  resolver->Reject(value);
}

void PromiseResolver::DoResolve(v8::Handle<v8::Value> value) {
  CHECK(LivePromiseResolverSet::instance()->IsLive(this));
  CHECK(runner_);
  auto const isolate = runner_->isolate();
  auto const resolver = resolver_.NewLocal(isolate);
  resolver->Resolve(value);
}

v8::Local<v8::Promise> PromiseResolver::GetPromise(v8::Isolate* isolate) const {
  CHECK(LivePromiseResolverSet::instance()->IsLive(this));
  auto const resolver = resolver_.NewLocal(isolate);
  return resolver->GetPromise();
}

void PromiseResolver::ScheduleRunMicrotasks() {
  CHECK(LivePromiseResolverSet::instance()->IsLive(this));
  if (type_ != Type::Fast)
    return;
  base::MessageLoop::current()->PostTask(
      FROM_HERE, base::Bind(&ScriptHost::RunMicrotasks,
                            base::Unretained(ScriptHost::instance())));
}

}  // namespace dom
