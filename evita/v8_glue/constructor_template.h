// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#if !defined(INCLUDE_evita_v8_glue_constructor_template_h)
#define INCLUDE_evita_v8_glue_constructor_template_h

// L3 C4191: 'operator/operation' : unsafe conversion from 'type of
// expression' to 'type required'
// L4 C4625: 'derived class' : copy constructor could not be generated because
// a base class copy constructor is inaccessible
// L4 C4626: 'derived class' : assignment operator could not be generated
// because a base class assignment operator is inaccessible
#pragma warning(push)
#pragma warning(disable: 4191 4625 4626)
#include "base/bind.h"
#pragma warning(pop)
#include "evita/v8_glue/function_template.h"

namespace v8_glue {

class AbstractScriptable;

namespace internal {

using gin::internal::CallbackHolderBase;
using gin::internal::CallbackHolder;

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
    v8::Handle<v8::External> v8_holder;
    CHECK(args.GetData(&v8_holder));
    CallbackHolderBase* holder_base = reinterpret_cast<CallbackHolderBase*>(
        v8_holder->Value());
    typedef CallbackHolder<R()> HolderT;
    HolderT* holder = static_cast<HolderT*>(holder_base);
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
    v8::Handle<v8::External> v8_holder;
    CHECK(args.GetData(&v8_holder));
    CallbackHolderBase* holder_base = reinterpret_cast<CallbackHolderBase*>(
        v8_holder->Value());
    typedef CallbackHolder<R(P1)> HolderT;
    HolderT* holder = static_cast<HolderT*>(holder_base);
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
    v8::Handle<v8::External> v8_holder;
    CHECK(args.GetData(&v8_holder));
    CallbackHolderBase* holder_base = reinterpret_cast<CallbackHolderBase*>(
        v8_holder->Value());
    typedef CallbackHolder<R(P1, P2)> HolderT;
    HolderT* holder = static_cast<HolderT*>(holder_base);
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
    v8::Handle<v8::External> v8_holder;
    CHECK(args.GetData(&v8_holder));
    CallbackHolderBase* holder_base = reinterpret_cast<CallbackHolderBase*>(
        v8_holder->Value());
    typedef CallbackHolder<R(P1, P2, P3)> HolderT;
    HolderT* holder = static_cast<HolderT*>(holder_base);
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

template<typename R, typename P1, typename P2, typename P3, typename P4>
struct ConstructorDispatcher<R(P1, P2, P3, P4)> {
  typedef v8::FunctionCallbackInfo<v8::Value> Info;
  typedef gin::internal::CallbackHolder<R(P1, P2, P3, P4)> Holder;

  static void DispatchToCallback(const Info& info) {
    gin::Arguments args(info);
    v8::Handle<v8::External> v8_holder;
    CHECK(args.GetData(&v8_holder));
    CallbackHolderBase* holder_base = reinterpret_cast<CallbackHolderBase*>(
        v8_holder->Value());
    typedef CallbackHolder<R(P1, P2, P3, P4)> HolderT;
    HolderT* holder = static_cast<HolderT*>(holder_base);
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
    typename gin::internal::CallbackParamTraits<P4>::LocalType a4;
    if (!gin::internal::GetNextArgument(&args, holder->flags, false, &a4)) {
      args.ThrowError();
      return;
    }
    auto impl = holder->callback.Run(a1, a2, a3, a4);
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
  auto holder = new Holder(isolate, callback, 0);
  return v8::FunctionTemplate::New(
      isolate,
      &internal::ConstructorDispatcher<Sig>::DispatchToCallback,
      gin::ConvertToV8<v8::Handle<v8::External>>(
          isolate, holder->GetHandle(isolate)));
}

template<typename CxxObject, typename... Params>
v8::Local<v8::FunctionTemplate> CreateConstructorTemplate(
    v8::Isolate* isolate,
    CxxObject (constructor)(Params...)) {
  return CreateConstructorTemplate(isolate, base::Bind(constructor));
}

}  // namespace v8_glue

#endif //!defined(INCLUDE_evita_v8_glue_constructor_template_h)
