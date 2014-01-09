// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
#if !defined(INCLUDE_evita_v8_glue_promise_resolver_h)
#define INCLUDE_evita_v8_glue_promise_resolver_h

#include <vector>

#include "base/memory/ref_counted.h"
#include "evita/v8_glue/converter.h"
#include "evita/v8_glue/scoped_persistent.h"
#include "evita/v8_glue/scriptable.h"

namespace v8_glue {

class PromiseResolver : public Scriptable<PromiseResolver> {
  protected: typedef std::vector<v8::Handle<v8::Value>> Argv;

  private: ScopedPersistent<v8::Value> reject_callback_;
  private: ScopedPersistent<v8::Value> resolve_callback_;

  public: PromiseResolver();
  public: ~PromiseResolver();

  public: static v8_glue::WrapperInfo* static_wrapper_info();

  private: v8::Handle<v8::Value> Call(v8::Isolate* isolate,
                                      v8::Handle<v8::Value> callback,
                                      const Argv& argv);
  private: static void PromiseResolver::CallAsFunctionHandler(
      const v8::FunctionCallbackInfo<v8::Value>& info);
  public: v8::Handle<v8::Value> NewPromise(v8::Isolate* isolate);
  public: template<typename T> void Reject(v8::Isolate* isolate, T value) {
    v8::HandleScope handle_scope(isolate);
    auto js_value = gin::Converter<T>::ToV8(isolate, value);
    Call(isolate, reject_callback_.NewLocal(isolate), {js_value});
  }
  public: template<typename T> void Resolve(v8::Isolate* isolate, T value) {
    v8::HandleScope handle_scope(isolate);
    auto js_value = gin::Converter<T>::ToV8(isolate, value);
    Call(isolate, resolve_callback_.NewLocal(isolate), {js_value});
  }

  public: v8::Handle<v8::Value> TestPromise();
  public: void TestResolve(v8::Handle<v8::Value> value);
  public: v8::Handle<v8::Value> onresolve() const;
};

}  // namespace v8_glue

#endif //!defined(INCLUDE_evita_v8_glue_promise_resolver_h)
