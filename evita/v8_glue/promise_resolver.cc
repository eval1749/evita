// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/v8_glue/promise_resolver.h"

#include "common/memory/singleton.h"
#include "evita/v8_glue/function_template_builder.h"

namespace v8_glue {

namespace {

//////////////////////////////////////////////////////////////////////
//
// PromiseConstructor
//
class PromiseConstructor : public common::Singleton<PromiseConstructor> {
  friend class common::Singleton<PromiseConstructor>;

  private: ScopedPersistent<v8::Value> value_;

  private: PromiseConstructor() = default;
  public: ~PromiseConstructor() = default;

  public: v8::Handle<v8::Value> Get(v8::Isolate* isoalte) const {
    return value_.NewLocal(isoalte);
  }

  public: static void Init(v8::Handle<v8::Value> value) {
    auto const isolate = v8::Isolate::GetCurrent();
    instance()->value_.Reset(isolate, value);
  }
};

//////////////////////////////////////////////////////////////////////
//
// PromiseResolverWrapperInfo
//
class PromiseResolverWrapperInfo : public WrapperInfo {
  public: PromiseResolverWrapperInfo()
      : WrapperInfo("PromiseResolver") {
  }
  public: ~PromiseResolverWrapperInfo() = default;

  private: virtual v8::Handle<v8::FunctionTemplate>
      CreateConstructorTemplate(v8::Isolate* isolate) override {
    auto templ = v8_glue::CreateConstructorTemplate(isolate, 
        &PromiseResolverWrapperInfo::NewPromiseResolver);
    return v8_glue::FunctionTemplateBuilder(isolate, templ)
        .SetMethod("init", &PromiseConstructor::Init)
        .Build();
  }

  private: static PromiseResolver* NewPromiseResolver() {
    return new PromiseResolver();
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("onresolve", &PromiseResolver::onresolve)
        .SetMethod("promise", &PromiseResolver::TestPromise)
        .SetMethod("promise2", &PromiseResolver::TestPromise2)
        .SetMethod("resolve", &PromiseResolver::TestResolve);
  }
};
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// PromiseResolver
//
PromiseResolver::PromiseResolver() {
}

PromiseResolver::~PromiseResolver() {
}

WrapperInfo* PromiseResolver::static_wrapper_info() {
  DEFINE_STATIC_LOCAL(PromiseResolverWrapperInfo, wrapper_info, ());
  return &wrapper_info;
}

v8::Handle<v8::Value> PromiseResolver::Call(v8::Isolate* isolate,
                                            v8::Handle<v8::Value> callee,
                                            const Argv& argv) {
  if (callee.IsEmpty() || !callee->IsObject() ||
      !callee->ToObject()->IsCallable()) {
    return v8::Undefined(isolate);
  }
  auto global = isolate->GetCurrentContext()->Global();
  return callee->ToObject()->CallAsFunction(global,
      argv.size(), const_cast<v8::Handle<v8::Value>*>(argv.data()));
}

void PromiseResolver::CallAsFunctionHandler(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  auto const isolate = info.GetIsolate();
  PromiseResolver* resolver = nullptr;
  CHECK(gin::ConvertFromV8(isolate, info.Data(), &resolver));
  if (info.Length() >= 1)
    resolver->resolve_callback_.Reset(isolate, info[0]);
  if (info.Length() >= 2)
    resolver->reject_callback_.Reset(isolate, info[1]);
}

v8::Handle<v8::Value> PromiseResolver::NewPromise(v8::Isolate* isolate) {
  #if 1
  auto js_promise = isolate->GetCurrentContext()->Global()->Get(
      gin::StringToV8(isolate, "Promise"));
  #else
  auto js_promise = PromiseConstructor::instance()->Get(isolate);
  #endif
  DCHECK(!js_promise.IsEmpty() && js_promise->IsObject() &&
         js_promise->ToObject()->IsCallable());
  auto const js_resolver = GetWrapper(isolate);
  auto const function = v8::Function::New(isolate,
    &PromiseResolver::CallAsFunctionHandler, js_resolver, 2);
  Argv argv {function};
  auto promise = js_promise.As<v8::Function>()->NewInstance(argv.size(),
      const_cast<v8::Handle<v8::Value>*>(argv.data()));
  DCHECK(promise->IsObject());
  DCHECK(promise->ToObject()->GetConstructor() == js_promise);
  return promise;
}

v8::Handle<v8::Value> PromiseResolver::TestPromise() {
  return NewPromise(v8::Isolate::GetCurrent());
}

void PromiseResolver::TestResolve(v8::Handle<v8::Value> value) {
  Resolve(v8::Isolate::GetCurrent(), value);
}

v8::Handle<v8::Value> PromiseResolver::onresolve() const {
  return resolve_callback_.NewLocal(v8::Isolate::GetCurrent());
}

v8::Handle<v8::Value> PromiseResolver::TestPromise2(v8::Handle<v8::Value> js_promise) {
  auto const isolate = v8::Isolate::GetCurrent();
  auto const js_resolver = GetWrapper(isolate);
  auto const function = v8::Function::New(isolate,
    &PromiseResolver::CallAsFunctionHandler, js_resolver, 2);
  Argv argv {function};
  return js_promise.As<v8::Function>()->NewInstance(argv.size(),
      const_cast<v8::Handle<v8::Value>*>(argv.data()));
}

}  // namespace v8_glue
