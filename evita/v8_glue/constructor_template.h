// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#if !defined(INCLUDE_evita_v8_glue_constructor_template_h)
#define INCLUDE_evita_v8_glue_constructor_template_h

#include "base/bind.h"
#include "evita/v8_glue/function_template.h"

namespace v8_glue {

class AbstractScriptable;

namespace internal {

void FinishConstructCall(const v8::FunctionCallbackInfo<v8::Value>& info,
                         AbstractScriptable* impl);
bool IsValidConstructCall(const v8::FunctionCallbackInfo<v8::Value>& info);

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
  typedef v8::FunctionCallbackInfo<v8::Value> Info;
  typedef gin::internal::CallbackHolder<R()> Holder;

  static void DispatchToCallback(const Info& info) {
    gin::Arguments args(info);
    gin::internal::CallbackHolderBase* holder_base = nullptr;
    CHECK(args.GetData(&holder_base));
    Holder* holder = static_cast<Holder*>(holder_base);
    if (!IsValidConstructCall(info))
      return;
    auto impl = holder->callback.Run();
    FinishConstructCall(info, impl);
  }
};

template<typename R, typename P1>
struct ConstructorDispatcher<R(P1)> {
  typedef v8::FunctionCallbackInfo<v8::Value> Info;
  typedef gin::internal::CallbackHolder<R(P1)> Holder;

  static void DispatchToCallback(const Info& info) {
    gin::Arguments args(info);
    gin::internal::CallbackHolderBase* holder_base = nullptr;
    CHECK(args.GetData(&holder_base));
    Holder* holder = static_cast<Holder*>(holder_base);
    if (!IsValidConstructCall(info))
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

template<typename R, typename P1, typename P2>
struct ConstructorDispatcher<R(P1, P2)> {
  typedef v8::FunctionCallbackInfo<v8::Value> Info;
  typedef gin::internal::CallbackHolder<R(P1, P2)> Holder;

  static void DispatchToCallback(const Info& info) {
    gin::Arguments args(info);
    gin::internal::CallbackHolderBase* holder_base = nullptr;
    CHECK(args.GetData(&holder_base));
    Holder* holder = static_cast<Holder*>(holder_base);
    if (!IsValidConstructCall(info))
      return;
    typename gin::internal::CallbackParamTraits<P1>::LocalType a1;
    if (!gin::internal::GetNextArgument(&args, holder->flags, true, &a1)) {
      args.ThrowError();
      return;
    }
    typename gin::internal::CallbackParamTraits<P2>::LocalType a2;
    if (!gin::internal::GetNextArgument(&args, holder->flags, false, &a2)) {
      args.ThrowError();
      return;
    }
    auto impl = holder->callback.Run(a1, a2);
    FinishConstructCall(info, impl);
  }
};

template<typename R, typename P1, typename P2, typename P3>
struct ConstructorDispatcher<R(P1, P2, P3)> {
  typedef v8::FunctionCallbackInfo<v8::Value> Info;
  typedef gin::internal::CallbackHolder<R(P1, P2, P3)> Holder;

  static void DispatchToCallback(const Info& info) {
    gin::Arguments args(info);
    gin::internal::CallbackHolderBase* holder_base = nullptr;
    CHECK(args.GetData(&holder_base));
    Holder* holder = static_cast<Holder*>(holder_base);
    if (!IsValidConstructCall(info))
      return;
    typename gin::internal::CallbackParamTraits<P1>::LocalType a1;
    if (!gin::internal::GetNextArgument(&args, holder->flags, true, &a1)) {
      args.ThrowError();
      return;
    }
    typename gin::internal::CallbackParamTraits<P2>::LocalType a2;
    if (!gin::internal::GetNextArgument(&args, holder->flags, false, &a2)) {
      args.ThrowError();
      return;
    }
    typename gin::internal::CallbackParamTraits<P3>::LocalType a3;
    if (!gin::internal::GetNextArgument(&args, holder->flags, false, &a3)) {
      args.ThrowError();
      return;
    }
    auto impl = holder->callback.Run(a1, a2, a3);
    FinishConstructCall(info, impl);
  }
};

}  // namespace internal

template<typename CxxObject, typename... Params>
v8::Local<v8::FunctionTemplate> CreateConstructorTemplate(
    v8::Isolate* isolate,
    const base::Callback<CxxObject(Params...)> callback) {
  typedef CxxObject (Sig)(Params...);
  typedef gin::internal::CallbackHolder<Sig> Holder;
  auto holder = gin::CreateHandle(isolate, new Holder(callback, 0));
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
