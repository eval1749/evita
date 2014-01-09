// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/v8_glue/promise_resolver.h"

namespace v8_glue {

namespace {
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
    return v8_glue::CreateConstructorTemplate(isolate, 
        &PromiseResolverWrapperInfo::NewPromiseResolver);
  }

  private: static PromiseResolver* NewPromiseResolver() {
    return new PromiseResolver();
  }

  private: virtual void SetupInstanceTemplate(
      ObjectTemplateBuilder& builder) override {
    builder
        .SetProperty("onresolve", &PromiseResolver::onresolve)
        .SetMethod("promise", &PromiseResolver::TestPromise)
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
  v8::TryCatch try_catch;
  auto value = callee->ToObject()->CallAsFunction(v8::Undefined(isolate),
      argv.size(), const_cast<v8::Handle<v8::Value>*>(argv.data()));
  if (try_catch.HasCaught()) {
    DVLOG(0) << "PromiseResolver::Call: failed.";
    try_catch.ReThrow();
  }
  base::string16 callee_text;
  gin::ConvertFromV8(isolate, callee->ToString(), &callee_text);
  base::string16 value_text;
  gin::ConvertFromV8(isolate, value->ToString(), &value_text);
  DVLOG(0) << "PromiseResolver::Call: value=" << value_text << " callee=" << callee_text;
  return value;
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
  auto js_promise = isolate->GetCurrentContext()->Global()->Get(
      gin::StringToV8(isolate, "Promise"));
  DCHECK(!js_promise.IsEmpty() && js_promise->IsObject() &&
         js_promise->ToObject()->IsCallable());
  auto const js_resolver = GetWrapper(isolate);
  auto const function = v8::Function::New(isolate,
    &PromiseResolver::CallAsFunctionHandler, js_resolver, 2);
  Argv argv {function};
  return js_promise->ToObject()->CallAsConstructor(argv.size(),
      const_cast<v8::Handle<v8::Value>*>(argv.data()));
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



}  // namespace v8_glue
