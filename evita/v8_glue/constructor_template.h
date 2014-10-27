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

using namespace gin::internal;

void FinishConstructCall(const v8::FunctionCallbackInfo<v8::Value>& info,
                         AbstractScriptable* impl);
bool IsValidConstructCall(const v8::FunctionCallbackInfo<v8::Value>& info);

// From http://crrev.com/671433004 Using indices technique.
template<typename T>
struct CallbackParamTraits {
  typedef T LocalType;
};
template<typename T>
struct CallbackParamTraits<const T&> {
  typedef T LocalType;
};
template<typename T>
struct CallbackParamTraits<const T*> {
  typedef T* LocalType;
};

// Classes for generating and storing an argument pack of integer indices
// (based on well-known "indices trick", see: http://goo.gl/bKKojn):
template <size_t... indices>
struct IndicesHolder {};

template <size_t requested_index, size_t... indices>
struct IndicesGenerator {
  using type = typename IndicesGenerator<requested_index - 1,
                                         requested_index - 1,
                                         indices...>::type;
};
template <size_t... indices>
struct IndicesGenerator<0, indices...> {
  using type = IndicesHolder<indices...>;
};

// Class template for extracting and storing single argument for callback
// at position |index|.
template <size_t index, typename ArgType>
struct ArgumentHolder {
  using ArgLocalType = typename CallbackParamTraits<ArgType>::LocalType;

  ArgLocalType value;
  bool ok;

  ArgumentHolder(gin::Arguments* args, int create_flags)
      : ok(GetNextArgument(args, create_flags, !index, &value)) {
    if (ok)
      return;
    args->ThrowError();
  }
};

//////////////////////////////////////////////////////////////////////
//
// Invoker
//

// Class template for converting arguments from JavaScript to C++ and running
// the callback with them.
template <typename IndicesType, typename... ArgTypes>
class Invoker {};

// Arguments are stored in member variable in base classes.
template <size_t... indices, typename... ArgTypes>
class Invoker<IndicesHolder<indices...>, ArgTypes...>
    : public ArgumentHolder<indices, ArgTypes>... {
  private: static bool And() { return true; }

  private: template <typename... T>
  static bool And(bool arg1, T... args) {
    return arg1 && And(args...);
  }

  private: gin::Arguments* args_;

  // Invoker<> inherits from ArgumentHolder<> for each argument.
  // C++ has always been strict about the class initialization order,
  // so it is guaranteed ArgumentHolders will be initialized (and thus, will
  // extract arguments from Arguments) in the right order.
  public: Invoker(gin::Arguments* args)
      : ArgumentHolder<indices, ArgTypes>(args, 0)...,
        args_(args) {
  }

  public: bool IsOK() {
    return And(ArgumentHolder<indices, ArgTypes>::ok...);
  }

  public: template <typename ReturnType>
  ReturnType DispatchToCallback(
      base::Callback<ReturnType(ArgTypes...)> callback) {
    return callback.Run(ArgumentHolder<indices, ArgTypes>::value...);
  }
};

//////////////////////////////////////////////////////////////////////
//
// Dispatcher
//

// This template catches unmatched signature.
template<typename Sig>
struct ConstructorDispatcher {
};

template<typename ReturnType, typename... ArgTypes>
struct ConstructorDispatcher<ReturnType(ArgTypes...)> {
  static void DispatchToCallback(
      const v8::FunctionCallbackInfo<v8::Value>& info) {
    gin::Arguments args(info);
    v8::Handle<v8::External> v8_holder;
    CHECK(args.GetData(&v8_holder));
    CallbackHolderBase* holder_base = reinterpret_cast<CallbackHolderBase*>(
        v8_holder->Value());
    typedef CallbackHolder<ReturnType(ArgTypes...)> HolderT;
    HolderT* holder = static_cast<HolderT*>(holder_base);
    if (!IsValidConstructCall(info))
      return;
    using Indices = typename IndicesGenerator<sizeof...(ArgTypes)>::type;
    Invoker<Indices, ArgTypes...> invoker(&args);
    if (!invoker.IsOK())
      return;
    auto const impl = invoker.DispatchToCallback(holder->callback);
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
