// Copyright (C) 2013 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#if !defined(INCLUDE_evita_v8_glue_call_as_function_handler_h)
#define INCLUDE_evita_v8_glue_call_as_function_handler_h

#include "base/bind.h"
#include "evita/v8_glue/function_template.h"

namespace v8_glue {

namespace internal {

template<typename R, typename... Params>
void SetCallAsFunctionHandler(
    v8::Isolate* isolate,
    v8::Handle<v8::ObjectTemplate> templ,
    const base ::Callback<R(Params...)> callback,
    int flags) {
  typedef R(Sig)(Params...);
  typedef gin::internal::CallbackHolder<Sig> Holder;
  auto holder = gin::CreateHandle(isolate, new Holder(callback, flags));
  templ->SetCallAsFunctionHandler(
      &gin::internal::Dispatcher<Sig>::DispatchToCallback,
      gin::ConvertToV8<gin::internal::CallbackHolderBase*>(
          isolate, holder.get()));
}

// Base template - used only for non-member function pointers. Other types
// either go to one of the below specializations, or go here and fail to compile
// because of base::Bind().
template<typename T, typename Enable = void>
struct CallAsFunctionTraits {
  static void Set(v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> templ,
                  T callback) {
    return SetCallAsFunctionHandler(isolate, base::Bind(callback));
  }
};

// Specialization for base::Callback<T>.
template<typename T>
struct CallAsFunctionTraits<base::Callback<T> > {
  static void Set(v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> templ,
                  const base::Callback<T>& callback) {
    return SetCallAsFunctionHandler(isolate, templ, callback, 0);
  }
};

// Specialization for member function pointers. We need to handle this case
// specially because the first parameter for callbacks to MFP should typically
// come from the the JavaScript "this" object the function was called on, not
// from the first normal parameter.
template<typename T>
struct CallAsFunctionTraits<T, typename base::enable_if<
                           base::is_member_function_pointer<T>::value>::type> {
  static void Set(v8::Isolate* isolate, v8::Handle<v8::ObjectTemplate> templ,
                  T callback) {
    return SetCallAsFunctionHandler(isolate, templ, base::Bind(callback),
                                    gin::HolderIsFirstArgument);
  }
};

}  // namespace internal

template<typename T>
void SetCallAsFunctionHandler(
    v8::Isolate* isolate,
    v8::Handle<v8::ObjectTemplate> templ,
    const T& callback) {
  internal::CallAsFunctionTraits<T>::Set(isolate, templ, callback);
}

}  // namespace v8_glue

#endif //!defined(INCLUDE_evita_v8_glue_call_as_function_handler_h)
