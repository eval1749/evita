// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#if !defined(INCLUDE_evita_v8_glue_constructor_template_h)
#define INCLUDE_evita_v8_glue_constructor_template_h

#include "base/bind.h"
BEGIN_V8_INCLUDE
#include "gin/function_template.h"
END_V8_INCLUDE

namespace v8_glue {

class AbstractScriptable;

namespace internal {

void FinishConstructCall(const v8::FunctionCallbackInfo<v8::Value>& info,
                         AbstractScriptable* impl);
bool IsValidConstructCall(const v8::FunctionCallbackInfo<v8::Value>& info,
                          int arity);

//////////////////////////////////////////////////////////////////////
//
// Dispatcher
//
// This tempalte catches unmatched signature.
template<typename Sig>
struct ConstructorDispatcher {
};

template<typename R>
struct ConstructorDispatcher<R()> {
  static void DispatchToCallback(
      const v8::FunctionCallbackInfo<v8::Value>& info) {
    gin::Arguments args(info);
    gin::internal::CallbackHolderBase* holder_base = NULL;
    CHECK(args.GetData(&holder_base));
    typedef gin::internal::CallbackHolder<R()> HolderT;
    HolderT* holder = static_cast<HolderT*>(holder_base);
    if (!IsValidConstructCall(info, 0))
      return;
    auto impl = holder->callback.Run();
    FinishConstructCall(info, impl);
  }
};

template<typename R, typename P1>
struct ConstructorDispatcher<R(P1)> {
  static void DispatchToCallback(
      const v8::FunctionCallbackInfo<v8::Value>& info) {
    gin::Arguments args(info);
    gin::internal::CallbackHolderBase* holder_base = NULL;
    CHECK(args.GetData(&holder_base));
    typedef gin::internal::CallbackHolder<R(P1)> HolderT;
    HolderT* holder = static_cast<HolderT*>(holder_base);
    if (!IsValidConstructCall(info, 1))
      return;
    typename gin::internal::CallbackParamTraits<P1>::LocalType a1;
    if (!gin::internal::GetNextArgument(&args, holder->flags, true, &a1)) {
      args.ThrowError();
      return;
    }
    auto impl = holder->callback.Run(a1);
    FinishConstructCall(info, impl);
  }
};

}  // namespace internal

template<typename CxxObject, typename... Params>
v8::Local<v8::FunctionTemplate> CreateConstructorTemplate(
    v8::Isolate* isolate,
    const base::Callback<CxxObject(Params...)> callback) {
  typedef CxxObject (Sig)(Params...);
  typedef gin::internal::CallbackHolder<Sig> HolderT;
  auto holder = gin::CreateHandle(isolate, new HolderT(callback, 0));
  return v8::FunctionTemplate::New(
      isolate,
      &internal::ConstructorDispatcher<Sig>::DispatchToCallback,
      gin::ConvertToV8<gin::internal::CallbackHolderBase*>(
          isolate, holder.get()));
}

template<typename CxxObject, typename... Params>
v8::Local<v8::FunctionTemplate> CreateConstructorTemplate(
    v8::Isolate* isolate,
    CxxObject (constructor)(Params...)) {
  return CreateConstructorTemplate(isolate, base::Bind(constructor));
}

}  // namespace v8_glue

#endif //!defined(INCLUDE_evita_v8_glue_constructor_template_h)
